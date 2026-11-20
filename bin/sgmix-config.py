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

# Configuration paths and platform labels.
CONFIG_FILE = Path("/opt/sgMIX/etc/sgmix.conf")
DEV_DIR = Path("/dev")
TREE_DIR = Path("/proc/device-tree")
SYS_VIDEO_DIR = Path("/sys/class/video4linux")
PLATFORMS = {
    "auto": "Auto detect",
    "nvidia": "NVIDIA",
    "rockchip": "Rockchip",
    "hobot": "Hobot / D-Robotics",
    "unknown": "Unknown",
}

# Product models offered by this tool, in menu order. Each name must exist
# in kCameraModels in SG_CameraDeviceTransfer.cpp.
MODELS = [
    "SG3S-ISX031C-GMSL2F-Hxxx",
    "SG3S-ISX031C-GMSL2-Hxxx",
    "SYF2G",
    "SYW2G",
    "SDT2BCO1",
    "SHW3G",
    "S56",
    "S56C",
    "S56D",
    "S56DY",
    "S36",
    "S36A",
    "SG3S11AFLK",
    "SHW5G",
    "SHF5G",
    "SYF3L",
    "SHF3L",
    "M3A",
    "SHF3H",
]

# Product model -> SIPL module name in the camera JSON.
SIPL_PRESET_BY_MODEL = {
    "SG3S-ISX031C-GMSL2-Hxxx": "SGX_YUV_GMSL2_3M",
    "SG3S-ISX031C-GMSL2F-Hxxx": "SGX_YUV_GMSL2_3M_3G",
    "SYF3L": "SGX_YUV_GMSL2_3M_3G",
    "SYF2G": "SGX_YUV_GMSL2_1920x1200",
    "SYW2G": "SGX_YUV_GMSL2_1920x1200",
    "SHF3L": "SGX_YUV_GMSL2_3M",
    "M3A": "SGX_YUV_GMSL2_3M",
    "SHF3H": "SGX_YUV_GMSL2_3M",
    "SG3S11AFLK": "SGX_YUV_GMSL2_3M_3G",
}


LEGACY_MODEL_NAMES = {"SHW2GY": "SYW2G", "SHF2GY": "SYF2G"}


def clean_text(value):
    return "".join(c if ord(c) >= 32 and c != "\x7f" else " " for c in str(value)).strip()


def normalize_model(value):
    if not value or value == "none":
        return ""
    value = LEGACY_MODEL_NAMES.get(value, value)
    if value in MODELS:
        return value
    raise ValueError("Unsupported camera model: " + clean_text(value))


def normalize_sipl_model(value):
    if not value or value == "none":
        return ""
    if clean_text(value) != value:
        raise ValueError("Invalid SIPL model name: " + clean_text(value))
    return normalize_model(value)


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
        model_name = name
        try:
            model = normalize_model(model_name)
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
            model_name = next(iter(drivers)) if drivers else name
            try:
                model = normalize_sipl_model(model_name)
            except ValueError:
                # The JSON module is usable after the user selects a product model.
                model = ""
        if model_name.lower() == "none":
            raise ValueError("SIPL JSON model name must not be none: " + name)
        rows.append({"port": (len(resolved) - 1 - transport) * 4 + link, "key": key, "module": name, "model": model, "mask": mask_text(masks)})
    return sorted(rows, key=lambda row: row["port"]), len(resolved)


def sipl_module_for_model(document, current, model):
    expected = SIPL_PRESET_BY_MODEL.get(model)
    if expected is None:
        return current
    definitions = [item for item in document.get("cameraConfigs", [])
                   if isinstance(item, dict) and item.get("name") == expected]
    current_definitions = [item for item in document.get("cameraConfigs", [])
                           if isinstance(item, dict) and item.get("name") == current]
    sgx_layout = current.startswith("SGX_YUV_GMSL2") or any(
        item.get("moduleDriverName") == "SGX_YUV_GMSL2" for item in current_definitions)
    if not definitions and not sgx_layout:
        # Dedicated vendor JSONs retain their native module definitions.
        return current
    if len(definitions) != 1 or "cameraModules" in definitions[0]:
        raise ValueError("Missing or ambiguous SIPL module mode: " + expected +
                         " (camera model " + model + ")")
    return expected


def sipl_mode_summary(document, name):
    definitions = [item for item in document.get("cameraConfigs", [])
                   if isinstance(item, dict) and item.get("name") == name]
    if len(definitions) != 1:
        return ""
    definition = definitions[0]
    modes = []
    for sensor in definition.get("sensorInfo", []):
        if not isinstance(sensor, dict):
            continue
        for vc in sensor.get("virtualChannels", []):
            if not isinstance(vc, dict) or not vc.get("width") or not vc.get("height"):
                continue
            text = "%sx%s" % (vc["width"], vc["height"])
            fps = vc.get("fps")
            if isinstance(fps, (int, float)) and not isinstance(fps, bool):
                text += " @ %g fps" % fps
            if vc.get("inputFormat"):
                text += " " + clean_text(vc["inputFormat"]).upper()
            if text not in modes:
                modes.append(text)
    rate = {"LINK_MODE_GMSL2_3GBPS": "3G", "LINK_MODE_GMSL2_6GBPS": "6G"}.get(
        definition.get("linkMode"))
    if rate:
        modes.append(rate)
    return ", ".join(modes)


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


CHECK_MARK, CROSS_MARK = "\u2713", "\u2717"
DEFAULT_THEME = 2  # Light


class UI:
    def __init__(self, force_text=False):
        self.terminal = None
        self.unsaved = False
        self.focus = {}
        self.last_directory = None
        self.theme = DEFAULT_THEME
        self.status = ""
        self.framework = "v4l2"
        self.filters = {}
        self.language = "en"
        self.settings_open = False
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
        # Keep this flag for the existing configuration/text-mode branches.
        self.dialog = bool(not force_text and self.terminal and sys.stdout.isatty() and sys.stderr.isatty()
                           and os.environ.get("TERM", "dumb") != "dumb")
        if self.terminal and not self.dialog:
            print("sgMIX configuration", file=sys.stderr)

    def page(self, text):
        print("\n" + "-" * 60, file=sys.stderr)
        print(text, file=sys.stderr)

    def menu(self, prompt, choices, default=None, states=None):
        if self.dialog:
            key = prompt.splitlines()[0]
            while True:
                entries, buttons, previous_group = [], [], None
                mode = self.filters.get(key, "All")
                for tag, label in choices:
                    tag = str(tag)
                    if states and tag in states and ((mode == "Enabled" and not states[tag]) or (mode == "Unset" and states[tag])):
                        continue
                    if tag == "back":
                        buttons.append(("back", "Keep editing" if label == "Keep editing" else "Back"))
                        continue
                    if tag in ("0", "q") and "Exit" in label:
                        buttons.append((tag, "Exit"))
                        continue
                    group = "Actions" if tag in ("s", "ports") else "Configuration"
                    if previous_group is not None and group != previous_group:
                        entries.append((None, ""))
                    previous_group = group
                    entries.append((tag, label + ("  [current]" if tag == str(default) else "")))
                if states and not any(tag in states for tag, _ in entries):
                    entries.insert(0, (None, "Nothing here. Change Filter."))
                selected = self.click_menu(prompt, entries, default if default is not None else self.focus.get(key),
                                           buttons=buttons or None, states=states)
                if selected == "__filter__":
                    modes = ("All", "Enabled", "Unset")
                    self.filters[key] = modes[(modes.index(mode) + 1) % len(modes)]
                    continue
                if selected is not None:
                    self.focus[key] = selected
                return selected
        # Text mode continues to require an explicit number/action.
        self.page(prompt)
        numbers = {str(tag) for tag, _ in choices if str(tag).isdigit()}
        aliases, next_number = {}, 1
        for tag, label in choices:
            tag = str(tag)
            if tag in ("back", "s"):
                key = "b" if tag == "back" else "s"
            elif tag.isdigit():
                key = tag
            else:
                while str(next_number) in numbers:
                    next_number += 1
                key = str(next_number)
                numbers.add(key)
                next_number += 1
            aliases[key] = tag
            print("  %s) %s" % (key, label), file=sys.stderr)
        while True:
            print("\nChoice: ", end="", file=sys.stderr, flush=True)
            value = self.read_line().strip()
            if value == ":cancel":
                return None
            if not value:
                print("Enter a number or action key.", file=sys.stderr)
            elif value in aliases:
                return aliases[value]
            elif value in dict(choices):
                return value
            else:
                print("Invalid choice. Use %s." % ", ".join(aliases), file=sys.stderr)

    def click_menu(self, prompt, entries, default=None, buttons=None, states=None):
        return self.panel(prompt, entries, default=default, buttons=buttons, states=states)

    def translate(self, text):
        if getattr(self, "language", "en") != "zh" or text is None:
            return text
        words = {
            "Back": "返回", "Exit": "退出", "Save": "保存", "Keep editing": "继续编辑",
            "Discard changes": "放弃修改", "Apply": "应用", "Continue": "继续", "Details": "详情",
            "All": "全选", "Clear": "清空", "All files": "全部文件", "JSON only": "仅 JSON",
            "Theme: Blue": "主题：蓝色", "Theme: Dark": "主题：黑色", "Theme: Light": "主题：浅色",
            "Settings": "设置", "sgMIX settings": "sgMIX 设置",
            "Language: English": "语言：English", "Language: 中文": "语言：中文",
            "Files: JSON only": "文件筛选：仅 JSON", "Files: All files": "文件筛选：全部",
            "Click an item to change it; Back returns.": "点击项目调整，返回后继续配置。",
            "Filter: All": "筛选：全部", "Filter: Enabled": "筛选：已启用", "Filter: Unset": "筛选：未配置",
            "Camera framework": "采集框架", "Camera model": "相机型号", "Review configuration": "保存预览",
            "Cameras - V4L2": "V4L2 相机", "Cameras - SIPL": "SIPL 通道",
            "Cameras - SIPL JSON file": "SIPL 配置文件", "Cameras - SIPL camera modules": "相机端口布局",
            "Select ports for batch model assignment": "批量设置相机型号", "Unsaved changes": "尚有未保存的修改",
            "sgMIX configuration": "sgMIX 配置", "Saved": "保存成功", "Cannot save": "无法保存",
            "V4L2 - video devices": "V4L2 - 视频设备", "SIPL - JSON configuration": "SIPL - JSON 配置",
            "Review and save": "预览并保存", "Batch model assignment": "批量设置型号",
            "Disable these ports": "禁用这些通道", "Clear model selection": "清除型号选择",
            "↑ Parent directory": "↑ 上一级", "Back to framework": "返回框架",
            "No model selected": "未选择型号",
            CHECK_MARK + " Enabled   " + CROSS_MARK + " Disabled": CHECK_MARK + " 已启用   " + CROSS_MARK + " 未启用",
            "Enabled": "已启用", "Disabled": "未启用", "Not configured": "未配置",
            "Nothing here. Change Filter.": "没有匹配通道，请切换筛选。",
            "Choose V4L2 or SIPL to continue.": "点击选择采集框架。",
            "Open a folder or click a JSON file.": "进入目录后，点击 JSON 文件。",
            "Choose the matching camera layout.": "选择与相机连接对应的端口布局。",
            "Click a model to assign it.": "点击型号即可应用到所选相机。",
            "Click a device to set its model.": "点击设备，设置对应的相机型号。",
            "No devices; save or go back.": "未发现设备，可保存或返回。",
            "Select ports, then click Apply.": "勾选通道后，点击应用。",
            "Choose a model or click a port status.": "可选择型号，也可点击通道状态开关。",
            "Click a port to apply the model. Click its [✓] again to disable.": "点击通道应用型号，再点 [✓] 可禁用。",
            "Choose a model for this CAM port first.": "请先为这个 CAM 口选择型号。",
            "CAM ports": "CAM 通道",
            "Cannot assign model": "无法设置型号",
            "Ports": "通道", "Models": "型号", "Model": "型号",
            "Check the settings, then Save.": "核对修改内容后，点击保存。",
            "Save, discard, or keep editing.": "请选择保存、放弃修改或继续编辑。",
            "Saved successfully. Click Continue.": "配置已保存，点击继续。",
            "Read the message, then Continue.": "阅读提示后，点击继续。",
            "Click an option to continue.": "点击选项继续。",
            "Scroll to read; Back returns.": "滚动查看完整内容，点击返回退出详情。",
            "Details shows the full configuration.": "点击详情可查看完整配置。",
            "Click Details to read the full selection.": "点击详情可查看完整名称和路径。",
            "SIPL files available": "SIPL 文件检查正常", "SIPL files found": "SIPL 文件检查正常",
            "SIPL files missing": "缺少 SIPL 文件", "Required SIPL files were found.": "已找到所需的 SIPL 文件。",
            "Required SIPL files are missing or unreadable.": "所需的 SIPL 文件缺失或无法读取。",
            "This checks file availability only. After saving, test camera initialization in your application.":
                "此处仅检查文件是否可用；保存后，请在应用中验证相机初始化。",
            "Install SIPL support matching this device before using SIPL.": "使用 SIPL 前，请安装与设备匹配的支持文件。",
            "You may continue configuring and save, or go back to choose V4L2.": "可继续配置并保存，也可返回选择 V4L2。",
            "No configuration changes.": "配置没有变化。",
            "Other changes are listed in Details.": "其他改动请查看详情。",
            "Create the initial configuration.": "创建初始配置。", "Configuration saved.": "配置已保存。",
            "No /dev/videoN nodes found.": "未发现 /dev/videoN 设备。",
            "Saving replaces the existing configuration.": "保存将替换现有配置。",
            "Saving creates this configuration file.": "保存将创建配置文件。",
            "Save to write these settings, or go back to keep editing.": "点击保存写入配置，或返回继续编辑。",
            "Enlarge terminal to 44 x 14. Esc: Back": "请将终端放大至 44 列 14 行，Esc 返回。",
            "Exited without saving. Existing configuration was not changed.": "已退出，未保存；原配置文件未改动。",
        }
        prefixes = {"File: ": "文件：", "JSON: ": "JSON：", "Directory: ": "目录：", "Current file: ": "当前文件：",
                    "Use configured JSON: ": "使用已配置的 JSON：", "Camera: ": "相机：", "Current model: ": "当前型号：",
                    "Source JSON: ": "源 JSON：", "Runtime JSON: ": "运行 JSON：", "Config: ": "配置：",
                    "Framework: ": "采集框架：", "Layout: ": "端口布局：", "Missing: ": "缺少：",
                    "Enabled masks: ": "启用掩码：", "Modules: ": "相机模块：", "[Directory] ": "[目录] "}
        result = []
        for line in str(text).split("\n"):
            padding = line[:len(line) - len(line.lstrip())]
            line = line[len(padding):]
            marker = "  [当前]" if line.endswith("  [current]") else ""
            if marker:
                line = line[:-11]
            if line in words:
                line = words[line]
            elif " | " in line and line.startswith("CAM"):
                parts = line.split(" | ")
                if len(parts) > 1:
                    parts[1] = words.get(parts[1], parts[1])
                if len(parts) > 2:
                    parts[2] = " -> ".join(words[model] if model == "No model selected" else model
                                          for model in parts[2].split(" -> "))
                line = " | ".join(parts)
            elif " -> " in line and line.startswith("/dev/"):
                device, model = line.split(" -> ", 1)
                line = device + " -> " + words.get(model, model)
            else:
                for prefix, translated in prefixes.items():
                    if line.startswith(prefix):
                        value = line[len(prefix):]
                        if prefix == "Current model: " and value == "No model selected":
                            value = words[value]
                        line = translated + value
                        break
            line = line.replace("  * Unsaved changes", "  * 未保存")
            if line.startswith("sgMIX | "):
                for english, chinese in (("Framework", "框架"), ("JSON file", "配置文件"), ("Cameras", "相机"), ("Review", "保存预览")):
                    line = line.replace(english, chinese)
            match = re.fullmatch(r"((?:video[0-9]+|CAM[0-9]+|T[0-9]+/L[0-3])) (model|enabled|module): (.*)", line)
            if match:
                target, field, values = match.groups()
                value_map = {"Not set": "未设置"}
                if field == "enabled":
                    value_map.update(yes="是", no="否")
                values = " -> ".join(value_map.get(value, value) for value in values.split(" -> "))
                line = target + " " + {"model": "型号", "enabled": "启用", "module": "模块"}[field] + "：" + values
            match = re.fullmatch(r"([0-9]+) selected", line)
            if match:
                line = "已选择 " + match[1] + " 项"
            result.append(padding + line + marker)
        return "\n".join(result)

    def step_label(self, prompt):
        title = prompt.splitlines()[0]
        sipl = getattr(self, "framework", "v4l2") == "sipl"
        total = 4 if sipl else 3
        if title == "Camera framework":
            return "sgMIX | 1/%d Framework" % total
        if title.startswith("Cameras - SIPL "):
            return "sgMIX | 2/4 JSON file"
        if title.startswith("Cameras - ") or title == "Camera model" or title.startswith("Select ports"):
            return "sgMIX | %d/%d Cameras" % (total - 1, total)
        if title == "Review configuration":
            return "sgMIX | %d/%d Review" % (total, total)
        return "sgMIX configuration"

    @staticmethod
    def page_hint(prompt, multiple=False, body=None, details=False):
        if prompt.splitlines()[0] == "Settings":
            return "Click an item to change it; Back returns."
        if details:
            return "Scroll to read; Back returns."
        if multiple:
            return "Select ports, then click Apply."
        title = prompt.splitlines()[0]
        if title == "Camera framework":
            return "Choose V4L2 or SIPL to continue."
        if title == "Cameras - SIPL JSON file":
            return "Open a folder or click a JSON file."
        if title == "Cameras - SIPL camera modules":
            return "Choose the matching camera layout."
        if title == "Camera model":
            return "Click a model to assign it."
        if title == "Cameras - V4L2":
            return "Click a device to set its model." if "No /dev/" not in prompt else "No devices; save or go back."
        if title == "Cameras - SIPL":
            return "Choose a model or click a port status."
        if title == "Review configuration":
            return "Check the settings, then Save."
        if title == "Unsaved changes":
            return "Save, discard, or keep editing."
        if title == "Saved":
            return "Saved successfully. Click Continue."
        return "Read the message, then Continue." if body is not None else "Click an option to continue."

    def panel(self, prompt, entries=(), default=None, multiple=False, body=None, buttons=None,
              detail_body=None, states=None, port_models=None, assign_port=None):
        """One renderer for menus, checklists, file browsing, messages and reviews."""
        import curses
        import unicodedata

        def display_text(text):
            return "\n".join("Camera setup" if line.startswith("Camera setup - ") else line
                             for line in str(text).splitlines() if not line.lstrip().startswith("Platform:"))

        prompt = display_text(prompt)
        body = display_text(body) if body is not None else None
        checkable = [tag for tag, _ in entries if tag is not None]
        model_tags = {}
        port_tags = set()
        if port_models:
            port_tags = set(checkable)
            model_tags = {"model:" + model: model for model, _ in port_models}
            entries = [("model:" + model, label) for model, label in port_models] + [(None, "CAM ports")] + list(entries)
        port_indexes = {tag: index for index, (tag, _) in enumerate(entries) if tag in port_tags}

        def show(screen):
            canvas = screen
            try:
                (CHECK_MARK + CROSS_MARK).encode(screen.encoding)
                mark_fallback = {}
            except UnicodeEncodeError:
                mark_fallback = str.maketrans({CHECK_MARK: "+", CROSS_MARK: "x"})
            normal, highlight, heading = 0, curses.A_REVERSE, curses.A_BOLD
            palettes = (("Blue", curses.COLOR_WHITE, curses.COLOR_BLUE, curses.COLOR_BLACK, curses.COLOR_CYAN, curses.COLOR_CYAN),
                        ("Dark", curses.COLOR_WHITE, curses.COLOR_BLACK, curses.COLOR_BLACK, curses.COLOR_CYAN, curses.COLOR_CYAN),
                        ("Light", curses.COLOR_BLACK, curses.COLOR_WHITE, curses.COLOR_WHITE, curses.COLOR_BLUE, curses.COLOR_BLUE))
            screen.keypad(True)
            try:
                curses.curs_set(0)
            except curses.error:
                pass
            curses.mouseinterval(0)
            curses.mousemask(curses.ALL_MOUSE_EVENTS | getattr(curses, "BUTTON5_PRESSED", 1 << 21))
            selectable = [i for i, (tag, _) in enumerate(entries) if tag is not None]
            position = next((n for n, i in enumerate(selectable) if entries[i][0] == default), 0)
            top, offset, checked, details = 0, 0, set(), False
            selected_model = None
            assignment_error = None
            settings = bool(getattr(self, "settings_open", False))
            self.settings_open = False
            settings_position, settings_top = 0, 0

            def put(y, x, text, style=0):
                try:
                    limit = max(1, canvas.getmaxyx()[1] - x - 1)
                    canvas.addstr(y, x, wrap(clean_text(text).translate(mark_fallback), limit)[0], style or normal)
                except curses.error:
                    pass

            def cells(text):
                return sum(0 if unicodedata.combining(char) else
                           (2 if unicodedata.east_asian_width(char) in ("W", "F") else 1) for char in text)

            def centered(y, text, style=0):
                line = wrap(clean_text(text), canvas.getmaxyx()[1] - 4)[0]
                put(y, max(2, (canvas.getmaxyx()[1] - cells(line)) // 2), line, style)

            def wrap(text, width):
                result = []
                for line in str(text).splitlines() or [""]:
                    part, occupied = "", 0
                    for char in line.expandtabs(4):
                        size = 0 if unicodedata.combining(char) else (2 if unicodedata.east_asian_width(char) in ("W", "F") else 1)
                        if occupied + size > max(1, width) and part:
                            if char.isspace():
                                result.append(part.rstrip())
                                part, occupied = "", 0
                                continue
                            split = part.rfind(" ")
                            if split > 0:
                                result.append(part[:split].rstrip())
                                part = part[split + 1:]
                                occupied = cells(part)
                            else:
                                result.append(part)
                                part, occupied = "", 0
                        part += char
                        occupied += size
                    result.append(part)
                return result

            while True:
                theme = getattr(self, "theme", DEFAULT_THEME) % len(palettes)
                name, foreground, background, selected_fg, selected_bg, accent = palettes[theme]
                if curses.has_colors():
                    curses.init_pair(1, foreground, background)
                    curses.init_pair(2, selected_fg, selected_bg)
                    curses.init_pair(3, accent, background)
                    normal = curses.color_pair(1)
                    highlight = curses.color_pair(2) | curses.A_BOLD
                    heading = curses.color_pair(3) | curses.A_BOLD
                    screen.bkgd(" ", normal)
                terminal_height, terminal_width = screen.getmaxyx()
                screen.erase()
                canvas = screen
                if terminal_height < 14 or terminal_width < 44:
                    put(0, 0, "Enlarge terminal to 44 x 14. Esc: Back")
                    screen.refresh()
                    if screen.getch() == 27:
                        if settings:
                            settings = False
                            continue
                        return None
                    continue
                # Redraw the outer frame at the current terminal bounds on every resize.
                screen.border()
                setting_items = []
                if curses.has_colors():
                    setting_items.append(("setting:theme", "Theme: " + name))
                setting_items.append(("setting:language", "Language: " + ("中文" if getattr(self, "language", "en") == "zh" else "English")))
                mode = "All"
                if states:
                    mode = self.filters.get(prompt.splitlines()[0], "All")
                    setting_items.append(("setting:__filter__", "Filter: " + mode))
                file_filter = next((label for action, label in buttons or [] if action == "filter"), None)
                if file_filter is not None:
                    setting_items.append(("setting:filter", "Files: JSON only" if file_filter == "All files" else "Files: All files"))
                if multiple:
                    setting_items.extend((("setting:all", "All"), ("setting:clear", "Clear")))
                items = setting_items if settings else entries
                if port_tags and not settings and mode != "All":
                    items = [(tag, label) for tag, label in entries
                             if tag not in port_tags or states[tag] == (mode == "Enabled")]
                    if not any(tag in port_tags for tag, _ in items):
                        items.append((None, "Nothing here. Change Filter."))
                indexes = [i for i, (tag, _) in enumerate(items) if tag is not None]
                if not settings and indexes:
                    position = min(position, len(indexes) - 1)
                first_port = next((n for n, i in enumerate(indexes) if items[i][0] in port_tags), None)
                view_position = settings_position if settings else position
                view_top = settings_top if settings else top
                text_mode = not settings and (body is not None or details)
                selected = indexes[view_position] if indexes else None
                display_entries = [(tag, self.translate(label)) for tag, label in items]
                full_label = display_entries[selected][1] if selected is not None else ""
                active_prompt = "Settings" if settings else ("Cannot assign model" if assignment_error else prompt)
                title, _, context = active_prompt.partition("\n")
                if active_prompt == "Cameras - SIPL":
                    context = CHECK_MARK + " Enabled   " + CROSS_MARK + " Disabled"
                title, context = self.translate(title), self.translate(context)
                hint = self.translate(self.page_hint(active_prompt, multiple and not settings,
                                                   body if not settings else None, details and not settings))
                if model_tags and not settings and not details:
                    hint = self.translate("Choose a model or click a port status." if selected_model is None
                                          else "Click a port to apply the model. Click its [✓] again to disable.")
                content = None if settings else ((detail_body if detail_body is not None else prompt + "\n\n" + full_label) if details else body)
                if details and assignment_error and not settings:
                    content = assignment_error
                content = self.translate(content)
                app_title = self.translate("sgMIX settings" if settings else self.step_label(prompt) +
                                           ("  * Unsaved changes" if getattr(self, "unsaved", False) else ""))
                available_width = terminal_width - 4
                available_height = terminal_height - 2
                preferred_width = max(48, cells(title) + 4, cells(hint) + 4,
                    cells(app_title) + 4,
                    max((cells(line) + 4 for line in context.splitlines()), default=0),
                    max((cells(label) + (4 if not settings and (multiple or tag in model_tags) else 0) + 12
                         for tag, label in display_entries if tag is not None), default=0) if not text_mode else 0,
                    max((cells(line) + 8 for line in str(content).splitlines()), default=0) if text_mode else 0)
                width = min(available_width, preferred_width)
                # Content determines the panel size; the terminal supplies its limits.
                title_lines = wrap(title, width - 4)
                summary = title_lines[:1] + wrap(hint, width - 4) + (wrap(context, width - 4) if context else [])
                visible_summary = summary[:3 if available_height >= 14 else 2]
                start = 4 + len(visible_summary)
                lines = wrap(content, width - 8) if text_mode else []
                height = min(available_height, max(12, start + (len(lines) if text_mode else len(items)) + 5))
                origin_y, origin_x = (terminal_height - height) // 2, (terminal_width - width) // 2
                canvas = curses.newwin(height, width, origin_y, origin_x)
                canvas.bkgd(" ", normal)
                regions = []
                if not settings:
                    label = "[" + self.translate("Settings") + "]"
                    settings_x = width - cells(label) - 2
                    put(0, settings_x, label, curses.A_BOLD)
                    regions.append((0, settings_x, settings_x + cells(label), "settings"))
                centered(1, app_title, curses.A_BOLD)
                for y, line in enumerate(visible_summary, 3):
                    centered(y, line, curses.A_BOLD if y == 3 else normal)
                capacity = max(1, height - start - 5)
                list_start = start
                if text_mode:
                    offset = min(max(0, offset), max(0, len(lines) - capacity))
                    body_width = max(cells(line) for line in lines)
                    body_x = (width - body_width) // 2
                    for n, line in enumerate(lines[offset:offset + capacity]):
                        put(start + n, body_x, line)
                    total, first, last = len(lines), offset, min(len(lines), offset + capacity)
                else:
                    if selected is not None:
                        if selected < view_top:
                            view_top = selected
                        elif selected >= view_top + capacity:
                            view_top = selected - capacity + 1
                    view_top = min(max(0, view_top), max(0, len(items) - capacity))
                    visible = display_entries[view_top:view_top + capacity]
                    list_start += max(0, (capacity - len(visible)) // 2)
                    block_width = min(width - 12, max((cells(label) + (4 if not settings and (multiple or tag in model_tags) else 0) for tag, label in visible if tag is not None), default=0))
                    block_x = max(2, (width - block_width) // 2)
                    for n, (tag, label) in enumerate(visible):
                        if tag is None:
                            if label:
                                centered(list_start + n, label, heading)
                            else:
                                put(list_start + n, block_x, "-" * block_width, heading)
                            continue
                        style = highlight if view_top + n == selected else normal
                        if tag in model_tags:
                            mark = CHECK_MARK if tag == selected_model else " "
                            label = "(" + mark + ") " + label
                        elif multiple and not settings:
                            mark = CHECK_MARK if tag in checked else CROSS_MARK
                            label = "[" + mark + "] " + label
                        label = wrap(label, max(1, block_width))[0]
                        if tag in port_tags:
                            prefix, separator, mark = label.partition(" | [")
                            if separator and mark[:2] in (CHECK_MARK + "]", CROSS_MARK + "]"):
                                status_x = block_x + cells(prefix) + 3
                                regions.append((list_start + n, status_x, status_x + 3, "toggle_port:" + tag))
                        put(list_start + n, block_x, label + " " * max(0, block_width - cells(label)), style)
                    total, first, last = len(items), view_top, min(len(items), view_top + capacity)
                if settings:
                    settings_top = view_top
                else:
                    top = view_top
                show_checked = multiple and not details and not settings
                if total > capacity or show_checked:
                    centered(height - 5, "%d-%d / %d%s" % (first + 1 if total else 0, last, total,
                             " | " + self.translate("%d selected" % len(checked)) if show_checked else ""))
                needs_details = not settings and not text_mode and (len(title_lines) > 1 or len(summary) > len(visible_summary) or
                    any(cells(label) + (4 if multiple or tag in model_tags else 0) > block_width for tag, label in visible if tag is not None))
                if detail_body is not None and not details and not settings:
                    needs_details = True
                if selected_model is not None and not settings and not details:
                    label = self.translate(dict(entries)[selected_model])
                    if model_tags[selected_model]:
                        label = self.translate("Model") + ": " + label
                    centered(height - 4, CHECK_MARK + " " + label, heading)
                elif needs_details:
                    centered(height - 4, self.translate("Details shows the full configuration." if detail_body is not None else "Click Details to read the full selection."))
                elif not settings and getattr(self, "status", ""):
                    centered(height - 4, self.translate(self.status), heading)
                actions = [("close_settings", "Back")] if settings else ([("close_details", "Back")] if details else
                    [(action, label) for action, label in (buttons or [("back", "Back")]) if action != "filter"])
                if multiple and not details and not settings:
                    actions = [("apply", "Apply")] + actions
                if model_tags and not text_mode and not settings and total > capacity:
                    actions.insert(0, ("jump_models", "Models"))
                    if first_port is not None:
                        actions.insert(1, ("jump_ports", "Ports"))
                if first:
                    put(start, width - 4, "[^]", curses.A_BOLD)
                    regions.append((start, width - 4, width - 1, "prev"))
                if last < total:
                    next_y = start + max(1, capacity - 1)
                    put(next_y, width - 4, "[v]", curses.A_BOLD)
                    regions.append((next_y, width - 4, width - 1, "next"))
                if needs_details:
                    actions.append(("details", "Details"))
                rows, row, used = [], [], 0
                for action, label in actions:
                    label = "[" + self.translate(label) + "]"
                    if row and used + 2 + cells(label) > width - 4:
                        rows.append(row)
                        row, used = [], 0
                    row.append((action, label))
                    used += cells(label) + (2 if len(row) > 1 else 0)
                if row:
                    rows.append(row)
                for n, row in enumerate(rows[:2]):
                    y = height - 3 + n
                    x = (width - sum(cells(label) for _, label in row) - 2 * (len(row) - 1)) // 2
                    for action, label in row:
                        put(y, x, label, curses.A_BOLD)
                        regions.append((y, x, x + cells(label), action))
                        x += cells(label) + 2
                screen.noutrefresh()
                canvas.noutrefresh()
                curses.doupdate()
                key, action, step = screen.getch(), None, 0
                if key == curses.KEY_MOUSE:
                    try:
                        _, mx, my, _, state = curses.getmouse()
                    except curses.error:
                        continue
                    mx, my = mx - origin_x, my - origin_y
                    if not (0 <= mx < width and 0 <= my < height):
                        continue
                    if state & curses.BUTTON4_PRESSED:
                        step = -1
                    elif state & getattr(curses, "BUTTON5_PRESSED", 1 << 21):
                        # Python 3.8 omits BUTTON5 constants with ncurses' ABI 2.
                        step = 1
                    elif state & (curses.BUTTON1_RELEASED | curses.BUTTON1_CLICKED):
                        action = next((a for ry, left, right, a in regions if my == ry and left <= mx < right), None)
                        if action and action.startswith("toggle_port:"):
                            action, chosen_tag = "toggle_sipl_port", action.split(":", 1)[1]
                        if action is None and not text_mode and block_x <= mx < block_x + block_width and list_start <= my < list_start + len(visible):
                            index = view_top + my - list_start
                            if index < len(items) and items[index][0] is not None:
                                tag = items[index][0]
                                if settings:
                                    settings_position = indexes.index(index)
                                    action = tag.split(":", 1)[1]
                                elif tag in model_tags or tag in port_tags:
                                    position = indexes.index(index)
                                    action, chosen_tag = "edit_sipl_port", tag
                                elif multiple:
                                    position = indexes.index(index)
                                    checked.symmetric_difference_update({tag})
                                else:
                                    return tag
                elif key in (27, ord('b'), ord('q')):
                    action = "close_settings" if settings else ("close_details" if details else "back")
                elif key in (10, 13, curses.KEY_ENTER, ord(' ')) and not text_mode and selected is not None:
                    tag = items[selected][0]
                    if settings:
                        action = tag.split(":", 1)[1]
                    elif tag in model_tags or tag in port_tags:
                        action = "toggle_sipl_port" if tag in port_tags and key == ord(' ') else "edit_sipl_port"
                        chosen_tag = tag
                    elif multiple:
                        checked.symmetric_difference_update({tag})
                    else:
                        return tag
                elif key in (curses.KEY_UP, curses.KEY_DOWN, curses.KEY_PPAGE, curses.KEY_NPAGE):
                    step = {curses.KEY_UP: -1, curses.KEY_DOWN: 1, curses.KEY_PPAGE: -capacity,
                            curses.KEY_NPAGE: capacity}[key]
                elif key == curses.KEY_HOME:
                    if settings:
                        settings_position = 0
                    else:
                        offset, position = 0, 0
                elif key == curses.KEY_END:
                    if settings:
                        settings_position = max(0, len(indexes) - 1)
                    else:
                        offset, position = max(0, total - capacity), max(0, len(indexes) - 1)
                if action == "settings":
                    settings, settings_position, settings_top = True, 0, 0
                    continue
                elif action == "close_settings":
                    settings = False
                    continue
                elif action == "theme":
                    self.theme = (theme + 1) % len(palettes)
                elif action == "language":
                    self.language = "zh" if getattr(self, "language", "en") == "en" else "en"
                elif action == "close_details":
                    details, offset = False, 0
                    assignment_error = None
                elif action == "details":
                    details, offset = True, 0
                elif action == "all":
                    checked = set(checkable)
                elif action == "clear":
                    checked.clear()
                elif action in ("edit_sipl_port", "toggle_sipl_port"):
                    if chosen_tag in model_tags:
                        selected_model = None if selected_model == chosen_tag else chosen_tag
                        if selected_model is not None and first_port is not None:
                            position = first_port
                            top = indexes[position]
                    elif action == "toggle_sipl_port" or selected_model is not None:
                        model = model_tags.get(selected_model)
                        try:
                            label, enabled = assign_port(chosen_tag, model, toggle=action == "toggle_sipl_port")
                        except (OSError, ValueError) as error:
                            assignment_error = str(error)
                            details, offset = True, 0
                        else:
                            entries[port_indexes[chosen_tag]] = (chosen_tag, label)
                            states[chosen_tag] = enabled
                    continue
                elif action == "apply":
                    return [tag for tag in checkable if tag in checked]
                elif action in ("jump_ports", "jump_models"):
                    position = first_port if action == "jump_ports" else 0
                    if action == "jump_models" and selected_model is not None:
                        position = next(n for n, i in enumerate(indexes) if items[i][0] == selected_model)
                    top = indexes[position]
                elif action == "back":
                    return None
                elif action in ("prev", "next"):
                    step = -capacity if action == "prev" else capacity
                elif action in ("__filter__", "filter"):
                    if port_tags and action == "__filter__":
                        modes = ("All", "Enabled", "Unset")
                        self.filters[prompt.splitlines()[0]] = modes[(modes.index(mode) + 1) % len(modes)]
                        position, top = 0, 0
                        continue
                    self.settings_open = settings
                    return action
                elif action is not None:
                    return action
                if text_mode:
                    offset += step
                elif indexes:
                    if settings:
                        settings_position = max(0, min(len(indexes) - 1, settings_position + step))
                    else:
                        position = max(0, min(len(indexes) - 1, position + step))
                    if abs(step) > 1:
                        if settings:
                            settings_top = indexes[settings_position] if step > 0 else max(0, indexes[settings_position] - capacity + 1)
                        else:
                            top = indexes[position] if step > 0 else max(0, indexes[position] - capacity + 1)

        try:
            return curses.wrapper(show)
        except KeyboardInterrupt:
            return None
        except curses.error as error:
            raise ValueError("Cannot display mouse UI: " + str(error)) from error

    def read_line(self):
        line = self.terminal.readline()
        if not line:
            raise EOFError
        return line.rstrip("\n")

    def input(self, prompt, default="", example=""):
        if self.dialog:
            return self.choose_file(prompt, default)
        self.page(prompt)
        if default:
            print("Current file: " + default + "\nEnter keeps the current file.", file=sys.stderr)
        print(example + "\nActions:\n  b) Back to framework selection\n  0) Exit without saving", file=sys.stderr)
        print("\nFile path or action: ", end="", file=sys.stderr, flush=True)
        value = self.read_line()
        if value.strip() in ("b", "back", ":cancel"):
            return None
        return value or default

    def choose_file(self, prompt, default=""):
        candidate = Path(default).expanduser().absolute() if default else (self.last_directory or Path.cwd())
        directory = candidate if candidate.is_dir() else candidate.parent
        while not directory.is_dir() and directory != directory.parent:
            directory = directory.parent
        all_files = False
        focus = "file:" + str(candidate)
        while True:
            try:
                paths = sorted(directory.iterdir(), key=lambda p: (not p.is_dir(), p.suffix.lower() != ".json", p.name.casefold()))
                entries = [("parent", "↑ Parent directory")] if directory != directory.parent else []
                if candidate.is_file():
                    entries.append(("file:" + str(candidate), "Use configured JSON: " + candidate.name))
                entries += [("dir:" + str(p) if p.is_dir() else "file:" + str(p),
                             "[Directory] " + p.name if p.is_dir() else str(p))
                            for p in paths if p.is_dir() or (p.is_file() and (all_files or p.suffix.lower() == ".json"))]
            except OSError as error:
                self.message("Cannot read directory:\n" + str(directory) + "\n" + str(error))
                if directory == directory.parent:
                    return None
                directory = directory.parent
                continue
            browser_prompt = prompt + "\nDirectory: " + str(directory)
            if candidate.is_file():
                browser_prompt += "\nCurrent file: " + str(candidate)
            result = self.panel(browser_prompt, entries, default=focus, buttons=[("back", "Back to framework"),
                                ("filter", "JSON only" if all_files else "All files"), ("exit", "Exit")])
            if result is None:
                return None
            if result == "exit":
                return "0"
            if result == "filter":
                all_files = not all_files
            elif result == "parent":
                focus = "dir:" + str(directory)
                directory = directory.parent
            elif result.startswith("dir:"):
                directory = Path(result[4:])
                focus = "parent"
            elif result.startswith("file:"):
                self.last_directory = Path(result[5:]).parent
                return result[5:]

    def message(self, text, title="sgMIX configuration"):
        if self.dialog:
            return self.panel(title, body=text, buttons=[("continue", "Continue")]) is not None
        print("\n" + text, file=sys.stderr)
        return True

    def review(self, text, full_text=None):
        if self.dialog:
            return self.panel("Review configuration", body=text,
                              buttons=[("save", "Save"), ("back", "Keep editing")], detail_body=full_text) == "save"
        self.page(text)
        while True:
            print("Save these settings? [y=Save / N=Keep editing]: ", end="", file=sys.stderr, flush=True)
            value = self.read_line().strip().lower()
            if value in ("y", "yes"):
                return True
            if value in ("", "n", "no", "b", "back", ":cancel"):
                return False
            print("Enter y to save, or n to keep editing.", file=sys.stderr)

    def edit_sipl_ports(self, rows, models, enabled, assign_model):
        by_port = {"port:" + str(row["port"]): row for row in rows}

        def port_label(row):
            key = row["key"]
            mark = CHECK_MARK if key in enabled else CROSS_MARK
            return "CAM%d | [%s] | %s" % (row["port"], mark, models.get(key) or "No model selected")

        def assign_port(tag, model, toggle=False):
            row = by_port[tag]
            key = row["key"]
            if key in enabled and (model is None or (toggle and model == models.get(key))):
                model = ""
            elif model is None:
                model = models.get(key)
                if not model:
                    raise ValueError(self.translate("Choose a model for this CAM port first."))
            assign_model({key}, model)
            return port_label(row), key in enabled

        entries = [(tag, port_label(row)) for tag, row in by_port.items()]
        states = {tag: row["key"] in enabled for tag, row in by_port.items()}
        choices = [(model, model) for model in MODELS]
        return self.panel("Cameras - SIPL", entries, states=states, port_models=choices, assign_port=assign_port,
                          buttons=[("s", "Review and save"), ("back", "Back"), ("q", "Exit")])

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
        self.sipl_document = {}
        self.checked = False
        self.baseline_framework = previous if previous in ("v4l2", "sipl") else "v4l2"
        self.baselines = {framework: self.snapshot(framework) for framework in ("v4l2", "sipl")}
        self.baselines["sipl"]["Source JSON"] = self.saved.get("SGMIX_SIPL_SOURCE_JSON", self.saved.get("SGMIX_SIPL_JSON", ""))
        self.baselines["sipl"]["Layout"] = self.saved.get("SGMIX_SIPL_CONFIG", "")
        self.note_change()

    def snapshot(self, framework=None):
        framework = framework or self.framework
        if framework == "v4l2":
            return {name + " model": model for name, model in self.video_models.items() if model}
        result = {"Source JSON": self.json_path, "Layout": self.config_name}
        for key in set(self.link_models) | self.enabled_links | set(self.link_modules):
            prefix = "T%d/L%d" % key
            result[prefix + " model"] = self.link_models.get(key, "")
            result[prefix + " enabled"] = "yes" if key in self.enabled_links else "no"
            result[prefix + " module"] = self.link_modules.get(key, "")
        return result

    @property
    def changed(self):
        return self.framework != self.baseline_framework or self.snapshot() != self.baselines[self.framework]

    def note_change(self):
        if self.ui is not None:
            self.ui.unsaved = self.changed
            self.ui.framework = self.framework

    def changes_text(self):
        before, after = self.baselines[self.framework], self.snapshot()
        lines = []
        if self.framework != self.baseline_framework:
            lines.append("Framework: %s -> %s" % (self.baseline_framework.upper(), self.framework.upper()))
        if self.framework == "sipl":
            if before["Layout"] != after["Layout"]:
                lines.append("Layout: " + after["Layout"])
            if before["Source JSON"] != after["Source JSON"]:
                lines.append("Source JSON: " + Path(after["Source JSON"]).name)
            lines.extend(self.sipl_port_preview(before, after))
        else:
            for key in sorted(set(before) | set(after)):
                old, new = before.get(key, ""), after.get(key, "")
                if old != new:
                    lines.append("%s: %s -> %s" % (key, old or "Not set", new or "Not set"))
        if not lines:
            if self.changed:
                lines.append("Other changes are listed in Details.")
            else:
                lines.append("No configuration changes." if CONFIG_FILE.exists() else "Create the initial configuration.")
        return "\n".join(lines)

    def sipl_port_preview(self, before, after):
        ports = {"T%d/L%d" % row["key"]: "CAM%d" % row["port"]
                 for row in sorted(self.rows or [], key=lambda row: row["port"])}
        for key, state in sorted(before.items()):
            if key.endswith(" enabled") and state == "yes":
                prefix, _ = key.rsplit(" ", 1)
                ports.setdefault(prefix, prefix)

        for prefix, label in ports.items():
            model_key = prefix + " model"
            old = before.get(model_key, "")
            new = after.get(model_key, "")
            enabled = after.get(prefix + " enabled") == "yes"
            model = new or "No model selected"
            if old and old != new:
                model = old + " -> " + model
            module_name = after.get(prefix + " module", "")
            mode = sipl_mode_summary(self.sipl_document, module_name) if enabled else ""
            yield "%s | %s | %s%s" % (label, CHECK_MARK if enabled else CROSS_MARK, model,
                                     " | " + mode if mode else "")

    def review_changes(self):
        if self.ui.dialog:
            full_text = self.review_text(details=True)
            return self.ui.review(self.changes_text(), full_text)
        return self.ui.review(self.review_text())

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
                    "This checks file availability only. After saving, "
                    "test camera initialization in your application.")
        if self.ui is not None:
            if not missing and getattr(self.ui, "dialog", False):
                self.ui.status = "SIPL files available"
            else:
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
                selected = "v4l2" if choice == "1" else "sipl"
                if selected != self.framework:
                    self.note_change()
                self.framework = selected
                if self.framework == "sipl":
                    self.notify_sipl()
                return "next"
            self.ui.message("Invalid framework selection.")

    def load_sipl(self, path, name, choose=False):
        path, document, configs = read_json(path)

        def resolve_legacy_layout(layout):
            # Older configs stored the module preset as the layout name.
            if len(configs) == 1 and layout in ("SGX_YUV_GMSL2_3M", "SGX_YUV_GMSL2_3M_3G"):
                return next(iter(configs))
            return layout

        name = resolve_legacy_layout(name)
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
        prior_path, prior_layout = self.loaded_source or (
            self.saved.get("SGMIX_SIPL_SOURCE_JSON", self.saved.get("SGMIX_SIPL_JSON", "")),
            self.saved.get("SGMIX_SIPL_CONFIG", ""),
        )
        if source != (prior_path, resolve_legacy_layout(prior_layout)):
            self.link_models.clear()
            self.enabled_links.clear()
            self.link_modules.clear()
        else:
            for row in rows:
                key = row["key"]
                if key in self.link_modules and self.link_modules[key] != row["module"]:
                    expected = sipl_module_for_model(document, row["module"], self.link_models.get(key))
                    # Earlier versions incorrectly assigned the 8M preset to SHW2GY.
                    old_shw2gy_mode = (self.link_models.get(key) == "SYW2G" and
                                      self.link_modules[key] == "SGX_YUV_GMSL2_8M" and
                                      expected == SIPL_PRESET_BY_MODEL["SYW2G"])
                    if self.link_modules[key] != expected and not old_shw2gy_mode:
                        self.link_models.pop(key, None)
                        self.enabled_links.discard(key)
        valid_keys = {row["key"] for row in rows}
        self.enabled_links.intersection_update(valid_keys)
        self.link_models = {key: value for key, value in self.link_models.items() if key in valid_keys}
        for row in rows:
            if row["key"] not in self.link_models:
                self.link_models[row["key"]] = row["model"]
        self.json_path, self.config_name = path, name
        self.loaded_source, self.rows, self.transport_count = source, rows, count
        self.sipl_document = document
        self.link_modules = {row["key"]: self.sipl_module(row) for row in rows}
        baseline = self.baselines["sipl"]
        original = baseline.get("Source JSON", "")
        same_layout = resolve_legacy_layout(baseline.get("Layout")) == name
        if original and str(Path(original).expanduser().resolve()) == path and same_layout:
            baseline["Source JSON"], baseline["Layout"] = path, name
            for row in rows:
                prefix = "T%d/L%d" % row["key"]
                baseline.setdefault(prefix + " model", row["model"])
                baseline.setdefault(prefix + " enabled", "no")
                if not baseline.get(prefix + " module"):
                    baseline[prefix + " module"] = row["module"]
        self.note_change()
        return True

    def choose_sipl_source(self):
        path = self.json_path
        while True:
            prompt = "Cameras - SIPL JSON file"
            if not path and not self.ui.dialog:
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
                previous_source = (self.json_path, self.config_name)
                if self.load_sipl(candidate, self.config_name, choose=True):
                    if previous_source != (self.json_path, self.config_name):
                        self.note_change()
                    return "next"
                path = str(Path(candidate).expanduser().resolve())
            except (OSError, ValueError, UnicodeError) as error:
                self.ui.message("Cannot read SIPL configuration:\n" + clean_text(error))

    def choose_model(self, label, model, detail=""):
        choices = [("0", "Disable these ports" if self.framework == "sipl" else "Clear model selection")] + [
            (str(i + 1), value)
            for i, value in enumerate(MODELS)]
        choices.append(("back", "Back to previous menu"))
        default = str(MODELS.index(model) + 1) if model in MODELS else "0"
        prompt = "Camera model\nCamera: " + label + "\nCurrent model: " + (model or "No model selected")
        if detail:
            prompt += "\n" + detail
        selected = self.ui.menu(prompt, choices, default)
        if selected is None or selected == "back":
            return None
        if selected not in dict(choices):
            raise ValueError("Invalid camera model selection.")
        return "" if selected == "0" else MODELS[int(selected) - 1]

    def configure_models(self):
        if self.framework == "sipl":
            return self.configure_sipl()
        while True:
            self.note_change()
            devices = {device.name: device for device in video_devices()}
            choices = [(name, str(device) + " -> " + (self.video_models.get(name) or "No model selected"))
                       for name, device in devices.items()]
            prompt = "Cameras - V4L2\nPlatform: " + PLATFORMS[self.detected]
            if not devices:
                prompt += "\nNo /dev/videoN nodes found."
            elif not self.ui.dialog:
                prompt += "\nChoose a device to set its model."
            choices.extend((("s", "Review and save"), ("back", "Back to framework selection"),
                            ("0", "Exit without saving")))
            selected = self.ui.menu(prompt, choices, None,
                                    states={name: bool(self.video_models.get(name)) for name in devices})
            if selected is None or selected == "back":
                return "back"
            if selected == "0":
                return "exit"
            try:
                if selected == "s":
                    if self.review_changes():
                        self.save()
                        return "saved"
                elif selected in devices:
                    model = self.choose_model(str(devices[selected]), self.video_models.get(selected, ""))
                    if model is not None:
                        if self.video_models.get(selected, "") != model:
                            self.note_change()
                        self.video_models[selected] = model
            except (OSError, ValueError) as error:
                self.ui.message(str(error))

    def sipl_summary(self):
        lines = ["Camera setup - " + PLATFORMS[self.detected], "", "Port   Driver   Camera model"]
        for row in self.rows or []:
            index = row["port"]
            model = self.link_models.get(row["key"], "")
            label = model if row["key"] in self.enabled_links else "Not configured"
            mode = sipl_mode_summary(self.sipl_document, self.sipl_module(row)) if row["key"] in self.enabled_links else ""
            lines.append("%-6d CAM%-5d %s%s" % (index, index, label, " | " + mode if mode else ""))
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
            self.note_change()
            prompt = (self.sipl_summary() +
                      "\n\nExample input: 0, 0 1, or 4-7"
                      "\ns: Review and save   b: Back   q: Exit without saving")
            if self.ui.dialog:
                action = self.ui.edit_sipl_ports(self.rows, self.link_models, self.enabled_links, self.assign_sipl_model)
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
                    if not self.review_changes():
                        continue
                    self.save()
                    return "saved"
                ports = self.port_selection(action)
                keys = {row["key"] for row in self.rows if row["port"] in ports}
                current = {self.link_models.get(key, "") for key in keys}
                model = self.choose_model("Ports " + ", ".join(map(str, ports)),
                                          next(iter(current)) if len(current) == 1 else "")
                if model is not None:
                    self.assign_sipl_model(keys, model)
            except (OSError, ValueError) as error:
                self.ui.message(str(error))

    def sipl_module(self, row):
        if row["key"] not in self.enabled_links:
            return row["module"]
        return sipl_module_for_model(self.sipl_document, row["module"], self.link_models.get(row["key"]))

    def assign_sipl_model(self, keys, model):
        selected = [row for row in self.rows or [] if row["key"] in keys]
        modules = {row["key"]: sipl_module_for_model(self.sipl_document, row["module"], model)
                   if model else row["module"] for row in selected}
        if model:
            # Validate the complete assignment before changing enabled ports or models.
            self.sipl_settings(modules)
            self.link_models.update(dict.fromkeys(keys, model))
            self.enabled_links.update(keys)
        else:
            self.enabled_links.difference_update(keys)
        self.link_modules.update(modules)
        self.note_change()

    def sipl_settings(self, modes=None):
        if modes is None:
            modes = {row["key"]: self.sipl_module(row) for row in self.rows or []
                     if row["key"] in self.enabled_links and self.link_models.get(row["key"])}
        if not modes:
            return self.json_path, None
        _, document, configs = read_json(self.json_path)
        if self.config_name not in configs:
            raise ValueError("SIPL port layout changed; reload the JSON before saving.")
        changed = False
        for key, name in modes.items():
            original = next((row["module"] for row in self.rows or [] if row["key"] == key), None)
            if original == name:
                continue
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
                                     prefix + "_MODULE=" + self.sipl_module(row), prefix + "_MODEL=" + model))
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
        self.baseline_framework = self.framework
        self.baselines = {framework: self.snapshot(framework) for framework in ("v4l2", "sipl")}
        if self.ui is not None:
            self.ui.unsaved = False
            print(self.ui.translate("Configuration saved.\nFile: " + str(CONFIG_FILE)))
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
            module_names = ", ".join(dict.fromkeys(self.sipl_module(row)
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
                enabled = row["key"] in self.enabled_links
                module = self.sipl_module(row)
                mode = sipl_mode_summary(self.sipl_document, module) if enabled else ""
                lines.append("  Port %d -> T%d/L%d | %s | %s | Module: %s%s" %
                             (index, *row["key"], CHECK_MARK if enabled else CROSS_MARK,
                              model or "No model selected", module,
                              " | " + mode if mode else ""))
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
            self.note_change()
            if step == "framework":
                result = self.choose_framework()
                if result == "back" and self.ui.dialog:
                    result = "exit"
                if result not in ("exit", "back") and pending_options:
                    self.apply_options()
                    self.note_change()
                    pending_options = False
                if result not in ("exit", "back"):
                    step = "source" if self.framework == "sipl" else "models"
            elif step == "source":
                result = self.choose_sipl_source()
                if result != "exit":
                    step = "models" if result == "next" else "framework"
            else:
                result = self.configure_models()
                if result == "saved":
                    return
                if result != "exit":
                    step = "source" if self.framework == "sipl" else "framework"
            if result == "exit":
                if self.ui.dialog and self.changed:
                    action = self.ui.menu("Unsaved changes",
                                          [("save", "Save"), ("discard", "Discard changes"),
                                           ("back", "Keep editing")], "back")
                    if action in (None, "back"):
                        continue
                    if action == "save":
                        try:
                            if self.review_changes():
                                self.save()
                                return
                        except (OSError, ValueError) as error:
                            self.ui.message(str(error), "Cannot save")
                        continue
                    if action != "discard":
                        continue
                break
        print(self.ui.translate("Exited without saving. Existing configuration was not changed."))

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
                    self.assign_sipl_model(keys, self.default_model)
                for assignment in self.args.sipl_model:
                    target, model = split_assignment(assignment)
                    if not re.fullmatch(r"[0-9]+:[0-3]", target):
                        raise ValueError("Use --sipl-model TRANSPORT:LINK=MODEL.")
                    key = tuple(map(int, target.split(":")))
                    if key not in keys:
                        raise ValueError("SIPL link not found: " + target)
                    self.assign_sipl_model({key}, normalize_sipl_model(model))
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
                        help="use mouse-enabled terminal UI (Python curses)")
    parser.add_argument("--framework", choices=("v4l2", "sipl"))
    parser.add_argument("--model", help="set the same model for all discovered targets; none clears them")
    parser.add_argument("--video-model", action="append", default=[], metavar="VIDEO=MODEL")
    parser.add_argument("--json", help="SIPL camera JSON file")
    parser.add_argument("--config", help="SIPL camera configuration name")
    parser.add_argument("--sipl-model", action="append", default=[], metavar="TRANSPORT:LINK=MODEL")
    args = parser.parse_args()
    if args.ui:
        try:
            import curses
        except ImportError:
            parser.error("--ui requires Python curses support.")
    interactive = args.ui or (len(sys.argv) == 1 and sys.stdin.isatty())
    ui = UI(force_text=not args.ui) if interactive else None
    if interactive and (ui.terminal is None or (args.ui and not ui.dialog)):
        parser.error("Menu mode requires an interactive terminal; TERM must not be dumb for --ui.")
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
