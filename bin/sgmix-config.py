#!/usr/bin/env python3
# sgMIX platform, framework and per-camera configuration.
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap

CONFIG_FILE = Path("/opt/sgMIX/etc/sgmix.conf")
DEV_DIR = Path("/dev")
TREE_DIR = Path("/proc/device-tree")
SYS_VIDEO_DIR = Path("/sys/class/video4linux")
# Keep this list aligned with kCameraModels in SG_CameraDeviceTransfer.cpp.
MODELS = [
    "SG3S-ISX031C-GMSL2F-Hxxx",
    "SG3S-ISX031C-GMSL2-Hxxx",
    "SHW2GY",
    "SDT2BCO1",
    "SHW3G",
    "S56",
    "S36",
    "GW5_DEVICE",  
    "GW6_DEVICE", 
    "EEPROM_CAM",
    # Additional product names.
    "SG3S11AFLK",
    "SHW5G",
    "SHF3L",
    "SHF3H",
    "SG8-ISX028C-G2G-Hxxx",
]
# Keep generic SDK modes compatible with saved/CLI settings, but hide them in menus.
HIDDEN_MENU_MODELS = {"GW5_DEVICE", "GW6_DEVICE", "EEPROM_CAM"}
SIPL_ISX031_CONFIGS = {
    "SG3S-ISX031C-GMSL2-Hxxx": "SGX_YUV_GMSL2_3M",
    "SG3S-ISX031C-GMSL2F-Hxxx": "SGX_YUV_GMSL2_3M_3G",
    "SG3-ISX031C-GMSL2-Hxxx": "SGX_YUV_GMSL2_3M",
    "SG3-ISX031C-GMSL2F-Hxxx": "SGX_YUV_GMSL2_3M_3G",
}
SIPL_ISX031_MODELS = set(SIPL_ISX031_CONFIGS) | {"SG3S11AFLK", "SHF3L", "SHF3H"}
MODEL_ALIASES = {"sdt2bc01": "SDT2BCO1",
                 "sg8s-ar0820c-5300-g2a-hxxx": "SHW2GY",
                 "isx028": "SG8-ISX028C-G2G-Hxxx"}
PLATFORMS = {"auto": "Auto detect", "nvidia": "NVIDIA", "rockchip": "Rockchip",
             "hobot": "Hobot / D-Robotics", "unknown": "Unknown"}


def clean_text(value):
    return "".join(c if ord(c) >= 32 and c != "\x7f" else " " for c in str(value)).strip()


def normalize_model(value):
    if not value or value.lower() == "none":
        return ""
    value = MODEL_ALIASES.get(value.lower(), value)
    for model in MODELS:
        if value.lower() == model.lower():
            return model
    raise ValueError("Unsupported camera model: " + clean_text(value))


def normalize_sipl_model(value):
    if not value or value.lower() == "none":
        return ""
    if clean_text(value) != value:
        raise ValueError("Invalid SIPL model name: " + clean_text(value))
    for model in SIPL_ISX031_CONFIGS:
        if value.lower() == model.lower():
            return model
    try:
        return normalize_model(value)
    except ValueError:
        return value


def read_config():
    result = {}
    try:
        for line in CONFIG_FILE.read_text(encoding="utf-8").splitlines():
            if line.lstrip().startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            result.setdefault(key.strip(), value.strip())
    except (OSError, UnicodeError):
        if CONFIG_FILE.exists():
            result["_read_status"] = "unreadable"
    return result


def detect_platform():
    values = []
    for name in ("compatible", "model"):
        try:
            values.append((TREE_DIR / name).read_bytes().replace(b"\0", b" ").decode("utf-8", "replace"))
        except OSError:
            values.append("")
    description = clean_text(values[1])
    data = " ".join(values).lower()
    if "nvidia" in data or "tegra" in data:
        return "nvidia", description
    if "rockchip" in data:
        return "rockchip", description
    if "hobot" in data or "d-robotics" in data:
        return "hobot", description
    return "unknown", description


def video_devices():
    # ezISP/main.cpp::GetVideoDevices: enumerate /dev/video* and sort numerically.
    try:
        devices = [p for p in DEV_DIR.iterdir() if re.fullmatch(r"video[0-9]+", p.name)]
    except OSError:
        devices = []
    return sorted(devices, key=lambda p: int(p.name[5:]))


def read_json(path):
    path = Path(path).expanduser().resolve()
    if clean_text(str(path)) != str(path):
        raise ValueError("SIPL JSON path must not contain control characters or surrounding whitespace.")
    if not path.exists():
        raise ValueError("SIPL JSON file not found: " + str(path))
    if not path.is_file():
        raise ValueError("SIPL JSON path is not a regular file: " + str(path))
    if path.stat().st_size > 4 * 1024 * 1024:
        raise ValueError("SIPL JSON must not exceed 4 MiB.")
    with path.open(encoding="utf-8") as stream:
        document = json.load(stream)
    if not isinstance(document, dict):
        raise ValueError("SIPL JSON must contain an object.")
    configs = document.get("cameraConfigs", [])
    if not isinstance(configs, list):
        raise ValueError("cameraConfigs must be an array.")
    systems = {}
    for config in configs:
        if not isinstance(config, dict) or "cameraModules" not in config:
            continue
        name = config.get("name")
        if not isinstance(name, str) or not name or clean_text(name) != name:
            raise ValueError("A camera configuration has an invalid name.")
        if name in systems:
            raise ValueError("Duplicate SIPL configuration name: " + name)
        systems[name] = config
    if not systems:
        raise ValueError("No GMSL camera configuration with cameraModules was found.")
    return str(path), document, systems


def sipl_rows(document, config):
    platforms = document.get("platformTransportSettings", [])
    if not isinstance(platforms, list):
        raise ValueError("platformTransportSettings must be an array.")
    candidates = []
    for platform in platforms:
        if isinstance(platform, dict) and isinstance(platform.get("transportSettings"), list):
            candidates.append(platform["transportSettings"])
    transports = config.get("transportSettings")
    if transports is None:
        if len(candidates) != 1:
            raise ValueError("The selected configuration must identify its transportSettings.")
        transports = candidates[0]
    if not isinstance(transports, list) or not 1 <= len(transports) <= 16:
        raise ValueError("The selected configuration needs 1 to 16 transports.")
    definitions = {}
    for group in candidates:
        for item in group:
            if isinstance(item, dict) and isinstance(item.get("name"), str):
                definitions.setdefault(item["name"], []).append(item)
    resolved = []
    for item in transports:
        if isinstance(item, str) or (isinstance(item, dict) and set(item) == {"name"}):
            name = item if isinstance(item, str) else item["name"]
            matches = definitions.get(name, [])
            if not matches or any(match != matches[0] for match in matches):
                raise ValueError("Missing or ambiguous transport: " + clean_text(name))
            item = matches[0]
        if (not isinstance(item, dict) or not isinstance(item.get("type", "GMSL"), str)
                or item.get("type", "GMSL").upper() != "GMSL"):
            raise ValueError("Mask configuration currently supports GMSL transports.")
        resolved.append(item)
    modules = config["cameraModules"]
    if not isinstance(modules, list) or not modules:
        raise ValueError("cameraModules must be a non-empty array.")
    module_definitions = {}
    for item in document.get("cameraConfigs", []):
        if isinstance(item, dict) and "cameraModules" not in item and isinstance(item.get("name"), str):
            module_definitions.setdefault(item["name"], []).append(item)
    rows, seen = [], set()
    for module in modules:
        if not isinstance(module, dict):
            raise ValueError("Each cameraModules entry must be an object.")
        transport, link, name = module.get("transportIndex"), module.get("linkIndex"), module.get("name")
        if type(transport) is not int or not 0 <= transport < len(resolved):
            raise ValueError("Invalid transportIndex in cameraModules.")
        if type(link) is not int or not 0 <= link <= 3:
            raise ValueError("Invalid linkIndex: expected 0 to 3.")
        if not isinstance(name, str) or not name or clean_text(name) != name:
            raise ValueError("Invalid module name in cameraModules.")
        key = (transport, link)
        if key in seen:
            raise ValueError("Duplicate camera module at T%d/L%d." % key)
        seen.add(key)
        masks = [0] * len(resolved)
        masks[transport] = 1 << (4 * link)
        try:
            model = normalize_model(name)
        except ValueError:
            drivers = set()
            for definition in module_definitions.get(name, []):
                if "moduleDriverName" in definition:
                    driver = definition["moduleDriverName"]
                    if not isinstance(driver, str) or not driver or clean_text(driver) != driver:
                        raise ValueError("Invalid moduleDriverName for " + name)
                    drivers.add(driver)
            if len(drivers) > 1:
                raise ValueError("Ambiguous moduleDriverName for " + name)
            model = normalize_sipl_model(next(iter(drivers)) if drivers else name)
        if not model:
            raise ValueError("SIPL JSON model name must not be none: " + name)
        rows.append({"port": (len(resolved) - 1 - transport) * 4 + link, "key": key, "module": name, "model": model, "mask": mask_text(masks)})
    return sorted(rows, key=lambda row: row["port"]), len(resolved)


def mask_text(masks):
    return " ".join("0x%04x" % mask for mask in masks)


def find_sipl_library(name):
    explicit = os.environ.get("SGMIX_SIPL_RUNTIME_LIBRARY_DIR")
    if explicit:
        paths = [Path(explicit) / name]
    else:
        paths = []
        if os.environ.get("LD_LIBRARY_PATH"):
            paths.extend(Path(part or ".") / name for part in os.environ["LD_LIBRARY_PATH"].split(":"))
        if shutil.which("ldconfig"):
            result = subprocess.run(["ldconfig", "-p"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
            for line in result.stdout.splitlines():
                parts = line.split()
                if parts and parts[0] == name:
                    paths.append(Path(parts[-1]))
        paths.extend(Path(directory) / name for directory in (
            "/usr/lib/aarch64-linux-gnu/nvidia", "/usr/lib/aarch64-linux-gnu",
            "/lib/aarch64-linux-gnu", "/usr/lib64", "/usr/lib", "/lib64", "/lib"))
    return any(path.is_file() and os.access(str(path), os.R_OK) for path in paths)


class UI:
    def __init__(self, force_text=False):
        self.terminal = None
        for stream in (sys.stdout, sys.stderr):
            if stream.isatty():
                try:
                    self.terminal = io.TextIOWrapper(open(os.ttyname(stream.fileno()), "r+b", buffering=0),
                                                     encoding="utf-8", line_buffering=True)
                    break
                except OSError:
                    pass
        if self.terminal is None:
            try:
                self.terminal = io.TextIOWrapper(open("/dev/tty", "r+b", buffering=0),
                                                 encoding="utf-8", line_buffering=True)
            except OSError:
                pass
        self.dialog = bool(not force_text and self.terminal and sys.stdout.isatty() and sys.stderr.isatty()
                           and os.environ.get("TERM", "dumb") != "dumb" and shutil.which("whiptail"))
        if self.terminal and not self.dialog:
            print("sgMIX configuration", file=sys.stderr)

    def run_dialog(self, arguments, title="sgMIX configuration"):
        result = subprocess.run(["whiptail", "--title", title, "--ok-button", "OK",
                                 "--cancel-button", "Cancel"] + arguments,
                                stdin=self.terminal, stdout=self.terminal, stderr=subprocess.PIPE, text=True)
        return result.stderr.rstrip("\n") if result.returncode == 0 else None

    def page(self, text):
        print("\n" + "-" * 60, file=sys.stderr)
        print(text, file=sys.stderr)

    def menu(self, prompt, choices, default):
        default = None
        def group(tag, label):
            return "Actions" if tag in ("s", "back") or (tag == "0" and "Exit" in label) else "Configuration"

        if self.dialog:
            size = shutil.get_terminal_size((80, 24))
            width, height = min(100, max(40, size.columns - 2)), min(24, max(12, size.lines - 2))
            lines = sum(max(1, len(textwrap.wrap(line, width - 6))) for line in prompt.splitlines())
            entries, headings = [], set()
            previous_group = None
            for tag, label in choices:
                current_group = group(str(tag), label)
                if current_group != previous_group:
                    heading = "__" + current_group.lower()
                    entries.append((heading, "=== " + current_group + " ==="))
                    headings.add(heading)
                    previous_group = current_group
                entries.append((str(tag), label))
            count = min(len(entries), max(1, height - lines - 7), 11)
            focus = str(default) if default is not None else str(choices[0][0])
            args = ["--default-item", focus, "--menu", prompt, str(height), str(width), str(count)]
            for tag, label in entries:
                args.extend((tag, label))
            while True:
                selected = self.run_dialog(args)
                if selected not in headings:
                    return selected
        default = None
        self.page(prompt)
        numbers = {str(tag) for tag, _ in choices if str(tag).isdigit()}
        aliases, labels = {}, {}
        next_number = 1
        previous_group = None
        for tag, label in choices:
            tag = str(tag)
            if tag == "back":
                key = "b"
            elif tag == "s":
                key = "s"
            elif tag.isdigit():
                key = tag
            else:
                while str(next_number) in numbers:
                    next_number += 1
                key = str(next_number)
                numbers.add(key)
                next_number += 1
            aliases[key] = tag
            labels[key] = label
            current_group = group(tag, label)
            if current_group != previous_group:
                print("\nActions:" if current_group == "Actions" else "", file=sys.stderr)
                previous_group = current_group
            print("  %s) %s" % (key, labels[key]), file=sys.stderr)
        default_key = next(key for key, tag in aliases.items() if tag == str(default)) if default is not None else None
        while True:
            prompt = "\nChoice [%s]: " % default_key if default_key is not None else "\nChoice: "
            print(prompt, end="", file=sys.stderr, flush=True)
            value = self.read_line().strip()
            if value == ":cancel":
                return None
            if not value:
                if default_key is not None:
                    return aliases[default_key]
                print("Enter a number or action key.", file=sys.stderr)
                continue
            if value in aliases:
                return aliases[value]
            # Existing tags remain accepted for scripts and experienced users.
            if value in dict(choices):
                return value
            print("Invalid choice. Use %s." % ", ".join(aliases), file=sys.stderr)

    def read_line(self):
        line = self.terminal.readline()
        if not line:
            raise EOFError
        return line.rstrip("\n")

    def input(self, prompt, default="", example=""):
        actions = "Actions:\n  b) Back to framework selection\n  0) Exit without saving"
        if self.dialog:
            value = self.run_dialog(["--cancel-button", "Back to framework selection", "--inputbox",
                                     prompt + "\n\n" + example + "\n\n" + actions, "16", "78", default])
        else:
            self.page(prompt)
            if default:
                print("Current file: " + default + "\nEnter keeps the current file.", file=sys.stderr)
            if example:
                print(example, file=sys.stderr)
            print("\n" + actions, file=sys.stderr)
            print("\n")
            print("\nFile path or action: ", end="", file=sys.stderr, flush=True)
            value = self.read_line()
        if value is None or value.strip() in ("b", "back", ":cancel"):
            return None
        if value.strip() == "0":
            return "0"
        return value or default

    def message(self, text, title="sgMIX configuration"):
        if self.dialog:
            button = "Continue" if title.startswith("SIPL files") else "OK"
            return self.run_dialog(["--ok-button", button, "--msgbox", text, "12", "78"], title) is not None
        print("\n" + text, file=sys.stderr)
        return True

    def review(self, text):
        if self.dialog:
            size = shutil.get_terminal_size((80, 24))
            return self.run_dialog(["--yes-button", "Save", "--no-button", "Back to previous menu", "--scrolltext", "--yesno", text,
                                    str(min(24, max(12, size.lines - 2))),
                                    str(min(100, max(40, size.columns - 2)))], "Review configuration") is not None
        self.page(text)
        while True:
            print("Save these settings? [y=Save / N=Keep editing]: ", end="", file=sys.stderr, flush=True)
            value = self.read_line().strip().lower()
            if value in ("y", "yes"):
                return True
            if value in ("", "n", "no", "b", "back", ":cancel"):
                return False
            print("Enter y to save, or n to keep editing.", file=sys.stderr)


class Configuration:
    def __init__(self, args, ui):
        self.args, self.ui = args, ui
        self.saved = read_config()
        self.detected, self.board = detect_platform()
        self.platform = "auto"
        previous = self.saved.get("SGMIX_FRAMEWORK", "").lower()
        self.framework = args.framework or (previous if previous in ("v4l2", "sipl") else "v4l2")
        self.saved_framework = previous if previous in ("v4l2", "sipl") else ("invalid" if previous else "not set")
        if self.saved.get("_read_status") == "unreadable":
            self.saved_framework = "unreadable"
        self.default_model = args.model if args.model is not None else ""
        self.video_models, self.link_models, self.link_modules = {}, {}, {}
        for key, value in self.saved.items():
            video = re.fullmatch(r"SGMIX_V4L2_(VIDEO[0-9]+)_MODEL", key)
            link = re.fullmatch(r"SGMIX_SIPL_T([0-9]+)_L([0-3])_MODEL", key)
            if not video and not link:
                continue
            try:
                model = normalize_model(value) if video else normalize_sipl_model(value)
            except ValueError:
                continue
            if video:
                self.video_models[video[1].lower()] = model
            else:
                self.link_models[tuple(map(int, link.groups()))] = model
        self.enabled_links = {key for key, model in self.link_models.items() if model}
        for key, value in self.saved.items():
            selected = re.fullmatch(r"SGMIX_SIPL_T([0-9]+)_L([0-3])_SELECTED_MODEL", key)
            if selected:
                try:
                    self.link_models[tuple(map(int, selected.groups()))] = normalize_sipl_model(value)
                except ValueError:
                    pass
        for key, value in self.saved.items():
            match = re.fullmatch(r"SGMIX_SIPL_T([0-9]+)_L([0-3])_MODULE", key)
            if match:
                self.link_modules[tuple(map(int, match.groups()))] = value
        self.json_path = args.json or self.saved.get("SGMIX_SIPL_SOURCE_JSON", self.saved.get("SGMIX_SIPL_JSON", ""))
        self.config_name = args.config or self.saved.get("SGMIX_SIPL_CONFIG", "")
        self.loaded_source, self.rows, self.transport_count = None, None, 0
        self.checked = False

    def notify_sipl(self):
        if self.checked:
            return
        missing = [name for name in ("libnvsipl.so", "libnvsipl_query.so") if not find_sipl_library(name)]
        if missing:
            title = "SIPL files missing"
            text = ("Required SIPL files are missing or unreadable.\n"
                    "Missing: " + ", ".join(missing) + "\n\n"
                    "Install SIPL support matching this device before using SIPL.\n"
                    "You may continue configuring and save, or go back to choose V4L2.")
        else:
            title = "SIPL files found"
            text = ("Required SIPL files were found.\n\n"
                    "This checks file availability only. After saving,\n"
                    "test camera initialization in your application.")
        if self.ui is not None:
            self.ui.message(text, title)
        else:
            print("\n" + text, file=sys.stderr)
        self.checked = True

    def effective_platform(self):
        return self.detected if self.platform == "auto" else self.platform

    def validate_framework(self):
        if self.framework == "sipl" and self.effective_platform() != "nvidia":
            raise ValueError("SIPL is available only for the NVIDIA platform. Use V4L2 on this platform.")

    def choose_framework(self):
        nvidia = self.effective_platform() == "nvidia"
        if not nvidia:
            self.framework = "v4l2"
        prompt = "Camera framework\nPlatform: " + PLATFORMS[self.effective_platform()]
        choices = [("1", "V4L2 - video devices")]
        if nvidia:
            choices.append(("2", "SIPL - JSON configuration"))
        else:
            prompt += "\nSIPL requires NVIDIA."
        choices.append(("0", "Exit without saving"))
        while True:
            choice = self.ui.menu(prompt, choices, "2" if self.framework == "sipl" else "1")
            if choice is None or choice == "back":
                return "back"
            if choice == "0":
                return "exit"
            if choice == "1" or (choice == "2" and nvidia):
                self.framework = "v4l2" if choice == "1" else "sipl"
                if self.framework == "sipl":
                    self.notify_sipl()
                return "next"
            self.ui.message("Invalid framework selection.")

    def load_sipl(self, path, name, choose=False):
        path, document, configs = read_json(path)
        # A saved single-module mode still uses the JSON's port layout in the menu.
        if name in SIPL_ISX031_CONFIGS.values() and len(configs) == 1:
            name = next(iter(configs))
        if choose and len(configs) > 1:
            names = list(configs)
            labels = []
            for config_name in names:
                modules = configs[config_name]["cameraModules"]
                if isinstance(modules, list):
                    module_names = list(dict.fromkeys(clean_text(module["name"]) for module in modules
                                                      if isinstance(module, dict) and isinstance(module.get("name"), str)))
                    label = ", ".join(module_names) or "Unnamed camera modules"
                    labels.append("%s (%d ports)" % (label, len(modules)))
                else:
                    labels.append("Invalid camera modules")
            label_counts = {}
            for label in labels:
                label_counts[label] = label_counts.get(label, 0) + 1
            lookup = {str(i + 1): value for i, value in enumerate(names)}
            choices = [(str(i + 1), names[i] + " - " + label)
                       for i, label in enumerate(labels)]
            choices.append(("back", "Back to previous menu"))
            default = str(names.index(name) + 1) if name in names else "1"
            selected = self.ui.menu("Cameras - SIPL camera modules\nJSON: " + path, choices, default)
            if selected is None or selected == "back":
                return False
            if selected not in lookup:
                raise ValueError("Invalid configuration selection.")
            name = lookup[selected]
        elif not name or choose:
            if len(configs) != 1:
                raise ValueError("Multiple SIPL configurations found; select one with --config.")
            name = next(iter(configs))
        if name not in configs:
            raise ValueError("SIPL configuration not found: " + clean_text(name) +
                             "; select a port layout with --config.")
        rows, count = sipl_rows(document, configs[name])
        source = (path, name)
        prior_source = self.loaded_source or (self.saved.get("SGMIX_SIPL_SOURCE_JSON", self.saved.get("SGMIX_SIPL_JSON", "")),
                                              self.saved.get("SGMIX_SIPL_CONFIG", ""))
        if prior_source[0] == path and prior_source[1] in SIPL_ISX031_CONFIGS.values() and len(configs) == 1:
            prior_source = source
        if source != prior_source:
            self.link_models.clear()
            self.enabled_links.clear()
            self.link_modules.clear()
        else:
            for row in rows:
                key = row["key"]
                if key in self.link_modules and self.link_modules[key] != row["module"]:
                    self.link_models.pop(key, None)
                    self.enabled_links.discard(key)
        valid_keys = {row["key"] for row in rows}
        self.enabled_links.intersection_update(valid_keys)
        self.link_models = {key: value for key, value in self.link_models.items() if key in valid_keys}
        for row in rows:
            if row["key"] not in self.link_models:
                self.link_models[row["key"]] = row["model"]
        self.link_modules = {row["key"]: row["module"] for row in rows}
        self.json_path, self.config_name = path, name
        self.loaded_source, self.rows, self.transport_count = source, rows, count
        return True

    def choose_sipl_source(self):
        path = self.json_path
        while True:
            prompt = "Cameras - SIPL JSON file"
            if not path:
                prompt += "\nEnter the path to your camera's SIPL JSON file."
            candidate = self.ui.input(prompt, path,
                                      example="Example: /home/user/camera_sipl.json")
            if candidate is None:
                return "back"
            if candidate == "0":
                return "exit"
            if not candidate.strip():
                self.ui.message("A JSON file path is required. Enter a path or go back.")
                continue
            try:
                if self.load_sipl(candidate, self.config_name, choose=True):
                    return "next"
                path = str(Path(candidate).expanduser().resolve())
            except (OSError, ValueError, UnicodeError) as error:
                self.ui.message("Cannot read SIPL configuration:\n" + clean_text(error))

    def choose_model(self, label, model, detail=""):
        # Keep interactive choices aligned with the SDK's fixed model registry.
        models = [value for value in MODELS if value not in HIDDEN_MENU_MODELS]
        choices = [("0", "Disable these ports" if self.framework == "sipl" else "Clear model selection")] + [
            (str(i + 1), value)
            for i, value in enumerate(models)]
        choices.append(("back", "Back to previous menu"))
        default = str(models.index(model) + 1) if model in models else "0"
        prompt = "Camera model\nCamera: " + label + "\nCurrent model: " + (model or "No model selected")
        if detail:
            prompt += "\n" + detail
        selected = self.ui.menu(prompt, choices, default)
        if selected is None or selected == "back":
            return None
        if selected not in dict(choices):
            raise ValueError("Invalid camera model selection.")
        return "" if selected == "0" else models[int(selected) - 1]

    def configure_models(self):
        if self.framework == "sipl":
            return self.configure_sipl()
        while True:
            devices = {device.name: device for device in video_devices()}
            choices = [(name, str(device) + " -> " + (self.video_models.get(name) or "No model selected"))
                       for name, device in devices.items()]
            prompt = "Cameras - V4L2\nPlatform: " + PLATFORMS[self.detected]
            prompt += "\nChoose a device to set its model." if devices else "\nNo /dev/videoN nodes found."
            choices.extend((("s", "Review and save"), ("back", "Back to framework selection"),
                            ("0", "Exit without saving")))
            selected = self.ui.menu(prompt, choices, None)
            if selected is None or selected == "back":
                return "back"
            if selected == "0":
                return "exit"
            try:
                if selected == "s":
                    if self.ui.review(self.review_text()):
                        self.save()
                        return "saved"
                elif selected in devices:
                    model = self.choose_model(str(devices[selected]), self.video_models.get(selected, ""))
                    if model is not None:
                        self.video_models[selected] = model
            except ValueError as error:
                self.ui.message(str(error))

    def sipl_summary(self):
        lines = ["Camera setup - " + PLATFORMS[self.detected], "", "Port   Driver   Camera model"]
        for row in self.rows or []:
            index = row["port"]
            model = self.link_models.get(row["key"], "")
            label = model if row["key"] in self.enabled_links else "Not configured"
            lines.append("%-6d CAM%-5d %s" % (index, index, label))
        return "\n".join(lines)

    def port_selection(self, value):
        # Normalize spaces around ranges before accepting either comma or space separators.
        value = re.sub(r"\s*-\s*", "-", value.strip())
        if not value or value.startswith(",") or value.endswith(",") or re.search(r",\s*,", value):
            raise ValueError("Enter ports such as 0 1, 0,1 or 4-7.")
        ports = set()
        for part in re.split(r"[,\s]+", value):
            match = re.fullmatch(r"([0-9]+)(?:-([0-9]+))?", part)
            if not match:
                raise ValueError("Enter ports such as 0 1, 0,1 or 4-7.")
            first, last = int(match[1]), int(match[2] or match[1])
            if not 0 <= first <= last < self.transport_count * 4:
                raise ValueError("Port range must be within 0-%d." % (self.transport_count * 4 - 1))
            available = {row["port"] for row in self.rows}
            if any(port not in available for port in range(first, last + 1)):
                raise ValueError("Port is not present in this JSON. Available ports: " +
                                 ", ".join(map(str, sorted(available))))
            ports.update(range(first, last + 1))
        return sorted(ports)

    def configure_sipl(self):
        while True:
            prompt = (self.sipl_summary() +
                      "\n\nExample input: 0, 0 1, or 4-7"
                      "\ns: Save and exit   b: Back   q: Exit without saving")
            if self.ui.dialog:
                action = self.ui.run_dialog(["--inputbox", prompt, "22", "78", ""])
            else:
                self.ui.page(prompt)
                print("\nPort number(s) or action: ", end="", file=sys.stderr, flush=True)
                action = self.ui.read_line()
            if action is None or action.strip() in ("b", "back", ":cancel"):
                return "back"
            action = action.strip().lower()
            if action == "q":
                return "exit"
            try:
                if action == "s":
                    if any(not self.link_models.get(key) for key in self.enabled_links):
                        raise ValueError("Select a camera model for every enabled port.")
                    self.save()
                    return "saved"
                ports = self.port_selection(action)
                keys = {row["key"] for row in self.rows if row["port"] in ports}
                model = self.choose_model("Ports " + ", ".join(map(str, ports)), "")
                if model is not None:
                    if model:
                        for key in keys:
                            self.link_models[key] = model
                        self.enabled_links.update(keys)
                    else:
                        self.enabled_links.difference_update(keys)
            except ValueError as error:
                self.ui.message(str(error))

    def sipl_settings(self):
        modes = {row["key"]: SIPL_ISX031_CONFIGS[self.link_models[row["key"]]] for row in self.rows or []
                 if row["key"] in self.enabled_links and self.link_models.get(row["key"]) in SIPL_ISX031_CONFIGS}
        if not modes:
            return self.json_path, None
        _, document, configs = read_json(self.json_path)
        if self.config_name not in configs:
            raise ValueError("SIPL port layout changed; reload the JSON before saving.")
        changed = False
        for key, name in modes.items():
            definitions = [item for item in document["cameraConfigs"]
                           if isinstance(item, dict) and item.get("name") == name]
            if len(definitions) != 1 or "cameraModules" in definitions[0]:
                raise ValueError("Missing or ambiguous SIPL module mode: " + name)
            for module in configs[self.config_name]["cameraModules"]:
                if (module["transportIndex"], module["linkIndex"]) == key:
                    changed |= module["name"] != name
                    module["name"] = name
        if not changed:
            return self.json_path, None
        data = (json.dumps(document, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
        if len(data) > 4 * 1024 * 1024:
            raise ValueError("Generated SIPL JSON must not exceed 4 MiB.")
        # Content-addressed copies keep the vendor JSON and an earlier saved config intact.
        path = CONFIG_FILE.parent / ("sgmix-sipl-" + hashlib.sha256(data).hexdigest() + ".json")
        return str(path), data

    def contents(self):
        self.validate_framework()
        platform = self.effective_platform()
        lines = ["SGMIX_PLATFORM=" + platform, "SGMIX_FRAMEWORK=" + self.framework]
        if self.framework == "v4l2":
            for device in video_devices():
                model = self.video_models.get(device.name, "")
                if model:
                    lines.append("SGMIX_V4L2_%s_MODEL=%s" % (device.name.upper(), model))
        elif self.rows is not None:
            runtime_path, data = self.sipl_settings()
            self.pending_json = (Path(runtime_path), data) if data is not None else None
            lines.extend(("SGMIX_SIPL_JSON=" + runtime_path, "SGMIX_SIPL_CONFIG=" + self.config_name))
            if runtime_path != self.json_path:
                lines.append("SGMIX_SIPL_SOURCE_JSON=" + self.json_path)
            masks = [0] * self.transport_count
            mappings = []
            for row in self.rows:
                transport, link = row["key"]
                model = self.link_models.get(row["key"], "")
                if model and row["key"] in self.enabled_links:
                    masks[transport] |= 1 << (4 * link)
                    prefix = "SGMIX_SIPL_T%d_L%d" % (transport, link)
                    mappings.extend((prefix + "_MASK=" + row["mask"],
                                     prefix + "_MODULE=" + row["module"], prefix + "_MODEL=" + model))
                elif row["key"] in self.link_models:
                    prefix = "SGMIX_SIPL_T%d_L%d" % (transport, link)
                    mappings.extend((prefix + "_MODULE=" + row["module"], prefix + "_MODEL=none",
                                     prefix + "_SELECTED_MODEL=" + (model or "none")))
            lines.append("SGMIX_SIPL_MASKS=" + mask_text(masks))
            lines.extend(mappings)
        return "\n".join(lines) + "\n"

    def save(self):
        contents = self.contents()
        if any("\0" in line or "\r" in line for line in contents.split("\n")):
            raise ValueError("Configuration values must not contain control characters.")
        CONFIG_FILE.parent.mkdir(parents=True, exist_ok=True)
        files = [self.pending_json] if self.framework == "sipl" and getattr(self, "pending_json", None) else []
        for path, data in files + [(CONFIG_FILE, contents.encode("utf-8"))]:
            if path != CONFIG_FILE and path.exists():
                if path.read_bytes() != data:
                    raise ValueError("Generated SIPL JSON contents do not match its name: " + str(path))
                continue
            descriptor, name = tempfile.mkstemp(prefix="." + path.name + ".", dir=str(path.parent))
            try:
                with os.fdopen(descriptor, "wb") as stream:
                    stream.write(data)
                    stream.flush()
                    os.fsync(stream.fileno())
                    os.fchmod(stream.fileno(), 0o644)
                os.replace(name, str(path))
            finally:
                if os.path.exists(name):
                    os.unlink(name)
        if self.ui is not None:
            print("Configuration saved.\nFile: " + str(CONFIG_FILE))
        else:
            print("Generated " + str(CONFIG_FILE))
            print(contents, end="")

    def camera_summary(self):
        if self.framework == "v4l2":
            cameras = [(str(device), self.video_models.get(device.name, "")) for device in video_devices()]
        elif self.rows is not None:
            cameras = [("Port %d" % row["port"], self.link_models.get(row["key"], ""))
                       for row in self.rows]
        else:
            return None
        return cameras

    def review_text(self, details=False):
        if self.framework == "sipl" and not details:
            self.sipl_settings()
            return self.sipl_summary() + "\nFile: " + str(CONFIG_FILE) + "\nOnly enabled ports will be initialized."
        platform = PLATFORMS[self.detected if self.platform == "auto" else self.platform]
        lines = ["Review and save", "File: " + str(CONFIG_FILE),
                 "Saving replaces the existing configuration." if CONFIG_FILE.exists() else "Saving creates this configuration file.",
                 "Platform: " + platform, "Framework: " + self.framework.upper()]
        if self.framework == "sipl":
            runtime_path, _ = self.sipl_settings() # Validate before asking to save.
            module_names = ", ".join(dict.fromkeys(SIPL_ISX031_CONFIGS.get(self.link_models.get(row["key"]), row["module"])
                                                    for row in self.rows or [] if self.link_models.get(row["key"])))
            lines.append("Source JSON: " + self.json_path)
            lines.append("Runtime JSON: " + runtime_path)
            lines.append("Config: " + self.config_name)
            masks = [0] * self.transport_count
            for row in self.rows or []:
                if row["key"] in self.enabled_links and self.link_models.get(row["key"]):
                    masks[row["key"][0]] |= 1 << (4 * row["key"][1])
            lines.append("Enabled masks: " + mask_text(masks))
            lines.append("Modules: " + module_names)
        cameras = self.camera_summary() or []
        count = sum(bool(model) for _, model in cameras)
        lines.append("Cameras with a model: %d of %d" % (count, len(cameras)))
        if self.framework == "sipl":
            for row in self.rows or []:
                index = row["port"]
                model = self.link_models.get(row["key"], "")
                route = "Flash" if model in SIPL_ISX031_MODELS else "EEPROM"
                lines.append("  Port %d -> T%d/L%d | %s | %s | Read path: %s" %
                             (index, *row["key"], "Enabled" if row["key"] in self.enabled_links else "Disabled",
                              model or "No model selected", route if row["key"] in self.enabled_links else "None"))
            lines.append("All enabled links are initialized, even when an application selects one port.")
        else:
            lines.extend("  %s -> %s" % (camera, model or "No model selected (omitted on save)") for camera, model in cameras)
        if not cameras:
            lines.append("No cameras detected; only platform and framework will be saved.")
        elif count < len(cameras):
            lines.append("Cameras without a model will not be included in the saved mappings.")
        lines.append("Save to write these settings, or go back to keep editing.")
        return "\n".join(lines)

    def menu(self):
        step = "framework"
        pending_options = bool(self.args.model is not None or self.args.video_model or
                               self.args.sipl_model or self.args.json or self.args.config)
        while True:
            if step == "framework":
                result = self.choose_framework()
                if result == "exit":
                    break
                if result == "back":
                    step = "framework"
                    continue
                if pending_options:
                    self.apply_options()
                    pending_options = False
                step = "source" if self.framework == "sipl" else "models"
            elif step == "source":
                result = self.choose_sipl_source()
                if result == "exit":
                    break
                step = "models" if result == "next" else "framework"
            else:
                result = self.configure_models()
                if result == "saved":
                    return
                if result == "exit":
                    break
                step = "source" if self.framework == "sipl" else "framework"
        print("Exited without saving. Existing configuration was not changed.")

    def apply_options(self):
        self.validate_framework()
        if self.framework == "v4l2":
            if self.args.json or self.args.config or self.args.sipl_model:
                raise ValueError("SIPL JSON and mask options require --framework sipl.")
            devices = {device.name for device in video_devices()}
            if self.args.model is not None:
                self.default_model = normalize_model(self.args.model)
                self.video_models.update((name, self.default_model) for name in devices)
            for assignment in self.args.video_model:
                node, model = split_assignment(assignment)
                node = Path(node).name
                if node not in devices:
                    raise ValueError("Video node not found: " + clean_text(node))
                self.video_models[node] = normalize_model(model)
        else:
            if self.args.video_model:
                raise ValueError("--video-model requires --framework v4l2.")
            self.notify_sipl()
            if self.json_path:
                self.load_sipl(self.json_path, self.config_name)
                keys = {row["key"] for row in self.rows}
                if self.args.model is not None:
                    self.default_model = normalize_sipl_model(self.args.model)
                    self.link_models.update((key, self.default_model) for key in keys)
                    self.enabled_links = set(keys) if self.default_model else set()
                for assignment in self.args.sipl_model:
                    target, model = split_assignment(assignment)
                    if not re.fullmatch(r"[0-9]+:[0-3]", target):
                        raise ValueError("Use --sipl-model TRANSPORT:LINK=MODEL.")
                    key = tuple(map(int, target.split(":")))
                    if key not in keys:
                        raise ValueError("SIPL link not found: " + target)
                    self.link_models[key] = normalize_sipl_model(model)
                    if self.link_models[key]:
                        self.enabled_links.add(key)
                    else:
                        self.enabled_links.discard(key)
            elif self.args.config or self.args.sipl_model or self.args.model is not None:
                raise ValueError("Select a SIPL JSON file with --json first.")
            else:
                print("No SIPL JSON selected; camera mappings were not generated.", file=sys.stderr)

    def direct(self):
        self.apply_options()
        self.save()


def split_assignment(value):
    if "=" not in value:
        raise ValueError("Expected TARGET=MODEL: " + clean_text(value))
    target, model = value.split("=", 1)
    if not target or not model:
        raise ValueError("Expected TARGET=MODEL; use none to clear a model.")
    return target, model


def main():
    parser = argparse.ArgumentParser(prog="sgmix-config.py",
                                     description="Configure platform, framework and individual cameras.",
                                     epilog="Camera models: " + ", ".join(MODELS))
    parser.add_argument("--ui", "--menu", dest="ui", action="store_true",
                        help="use dialog boxes instead of the default terminal prompts")
    parser.add_argument("--framework", choices=("v4l2", "sipl"))
    parser.add_argument("--model", help="set the same model for all discovered targets; none clears them")
    parser.add_argument("--video-model", action="append", default=[], metavar="VIDEO=MODEL")
    parser.add_argument("--json", help="SIPL camera JSON file")
    parser.add_argument("--config", help="SIPL camera configuration name")
    parser.add_argument("--sipl-model", action="append", default=[], metavar="TRANSPORT:LINK=MODEL")
    args = parser.parse_args()
    interactive = args.ui or (len(sys.argv) == 1 and sys.stdin.isatty())
    ui = UI(force_text=not args.ui) if interactive else None
    if interactive and ui.terminal is None:
        parser.error("Menu mode requires a terminal.")
    try:
        config = Configuration(args, ui)
        if config.detected == "unknown":
            raise ValueError("Platform detection failed. No configuration was changed. Check /proc/device-tree model and compatible.")
        if interactive:
            config.menu()
        else:
            config.direct()
    except EOFError:
        print("\nInput closed. Configuration was not saved.", file=sys.stderr)
        return 0
    except KeyboardInterrupt:
        print("\nConfiguration interrupted.", file=sys.stderr)
        return 130
    except (OSError, ValueError, UnicodeError) as error:
        print("Configuration failed: " + clean_text(error), file=sys.stderr)
        return 1
    finally:
        if ui and ui.terminal:
            ui.terminal.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
