#!/bin/bash
# Install NVIDIA GStreamer plugins for the currently installed Jetson BSP.
set -Eeuo pipefail
export LC_ALL=C

plugin=nvidia-l4t-gstreamer
unit=sgmix-gst-install.service
wait_timeout=600
worker_dir=/run/sgmix
frontend_lock=/var/lib/dpkg/lock-frontend
dpkg_lock=/var/lib/dpkg/lock
gst_tmp=''
declare -A installed=() selections=()

die() { echo "sgMIX GST: $*" >&2; exit 1; }
cleanup() { [[ $gst_tmp != /tmp/sgmix-gst.* ]] || rm -rf -- "$gst_tmp"; }
trap cleanup EXIT

protected() {
    case "${1%%:*}" in
        nvidia-l4t-kernel*|nvidia-l4t-display-kernel*|nvidia-l4t-bootloader*|nvidia-l4t-initrd*|nvidia-l4t-extlinux*|nvidia-l4t-firmware*|nvidia-l4t-bsp-*|nvidia-igx-*|linux-*|flash-kernel|initramfs-tools*|grub*|shim*|nvidia-driver*|nvidia-dkms*|nvidia-kernel*|nvidia-firmware*|cuda-drivers*) return 0;;
        *) return 1;;
    esac
}

sdk_state() {
    local status code
    if status=$(dpkg-query -W -f='${db:Status-Status}' sgmix-sdk 2>/dev/null); then :; else
        code=$?; [[ $code == 1 ]] && return 1
        die 'Cannot read the SDK package database'
    fi
    case "$status" in config-files|not-installed) return 1;; esac
    [[ ${1:-} == schedule || $status == installed ]] || die "SDK is not configured: $status"
}

snapshot() {
    dpkg-query -W -f='${Package}:${Architecture}\t${Version}\t${db:Status-Status}\t${db:Status-Want}\n' |
        awk -F '\t' '$3 == "installed" {print $1 "\t" $2 "\t" $4}'
}

load_snapshot() {
    local key version selection
    installed=(); selections=()
    while IFS=$'\t' read -r key version selection; do
        installed["$key"]=$version; selections["$key"]=$selection
    done < "$1"
}

verify_state() {
    local key version selection current
    sdk_state || die 'SDK was removed after planning'
    while IFS=$'\t' read -r key version selection; do
        if [[ $key == nvidia-l4t-* ]] || protected "$key"; then
            current=$(dpkg-query -W -f='${db:Status-Status}|${Version}|${db:Status-Want}' "$key" 2>/dev/null || true)
            [[ $current == "installed|$version|$selection" ]] || die "Existing BSP package changed: $key"
        fi
    done < "$1/installed"
}

validate_package() {
    local key=$1 version=$2 name previous
    name=${key%%:*}; [[ $key == *:* ]] || key+=:arm64
    [[ $name != sgmix-sdk ]] || die 'The plugin task must not reinstall the SDK'
    protected "$name" && die "Refusing kernel/boot/firmware action: $name=$version"
    if [[ $name == nvidia-l4t-* ]]; then
        [[ $version == "$core" ]] || die "BSP mismatch: $name=$version; core=$core"
        previous=${installed["$key"]-}
        if [[ -n $previous ]]; then
            [[ $name == "$plugin" && $previous == "$version" ]] || die "Refusing an existing BSP package change: $name"
            [[ ${selections["$key"]-} != hold ]] || die "Plugin package is held; refusing to modify it: $name"
        fi
    fi
    return 0
}

approve_plan() {
    local package version key
    awk '
        /^Remv / {print "Refusing package removal: " $0 > "/dev/stderr"; exit 1}
        /^Inst / {
            name=$2; text=$0
            if (!sub(/^[^(]*\(/,"",text)) {print "Unrecognized APT plan: " $0 > "/dev/stderr"; exit 1}
            split(text, fields, /[[:space:]]+/)
            print name "\t" fields[1]
        }' "$1/plan" > "$1/actions" || die 'APT plan rejected'
    : > "$1/approved"
    while IFS=$'\t' read -r package version; do
        validate_package "$package" "$version"
        key=$package; [[ $key == *:* ]] || key+=:arm64
        printf '%s\t%s\n' "$key" "$version" >> "$1/approved"
    done < "$1/actions"
}

archive_guard() {
    local state=$1 archive name version architecture key alternate
    core=$(<"$state/core"); load_snapshot "$state/installed"; verify_state "$state"
    while IFS= read -r archive; do
        [[ -n $archive ]] || continue
        name=$(dpkg-deb -f "$archive" Package)
        version=$(dpkg-deb -f "$archive" Version)
        architecture=$(dpkg-deb -f "$archive" Architecture)
        key=$name:$architecture; alternate=$key
        [[ $architecture != all ]] || alternate=$name:arm64
        awk -F '\t' -v key="$key" -v alt="$alternate" -v version="$version" '
            ($1==key || $1==alt) && $2==version {found=1} END {exit !found}' "$state/approved" || die "Archive differs from approved plan: $name=$version"
        validate_package "$key" "$version"
    done
}

wait_for_apt() {
    local deadline=$((SECONDS + wait_timeout)) path identity device inode key busy announced=0
    [[ -r /proc/locks ]] || die 'Cannot read the kernel lock table'
    while true; do
        busy=0
        for path in "$frontend_lock" "$dpkg_lock"; do
            [[ -e $path ]] || continue
            identity=$(stat -Lc '%d %i' "$path") || die 'Cannot inspect the package-manager lock file'
            read -r device inode <<< "$identity"
            # /proc/locks identifies files by Linux device major:minor and inode.
            printf -v key '%02x:%02x:%s' "$(( ((device >> 8) & 0xfff) | ((device >> 32) & 0xfffff000) ))" \
                "$(( (device & 0xff) | ((device >> 12) & 0xffffff00) ))" "$inode"
            if awk -v key="$key" '$2 != "->" && $6 == key {held=1} END {exit !held}' /proc/locks; then busy=1; fi
        done
        (( busy )) || return 0
        if (( !announced )); then echo 'Waiting for the current APT/dpkg transaction to finish.'; announced=1; fi
        (( SECONDS < deadline )) || die 'Timed out waiting for APT/dpkg; no packages were installed'
        sleep 1
    done
}

systemd_available() { [[ -d /run/systemd/system ]] && command -v systemd-run >/dev/null; }

schedule() {
    local state
    sdk_state schedule || return 0
    if plugins_ready; then
        echo 'NVIDIA GStreamer plugins are already available.'
        return 0
    fi
    systemd_available || die 'No running systemd; run this script manually after APT exits'
    state=$(systemctl show "$unit" -p SubState --value 2>/dev/null || true)
    case "$state" in running|start|start-pre|start-post|stop|stop-sigterm|stop-sigkill)
        echo 'NVIDIA plugin task is already scheduled/running.'; return 0;;
        exited) systemctl stop "$unit";;
    esac
    systemctl reset-failed "$unit" >/dev/null 2>&1 || true
    systemd-run --quiet --no-block --unit="$unit" --description='sgMIX NVIDIA GStreamer plugins' \
        --property=Type=oneshot --property=RemainAfterExit=yes --property=TimeoutStartSec=infinity \
        /bin/bash "$(readlink -f "${BASH_SOURCE[0]}")"
    echo "NVIDIA plugins install in background; status: systemctl status $unit"
}

plugins_ready() {
    command -v gst-inspect-1.0 >/dev/null && gst-inspect-1.0 nvarguscamerasrc >/dev/null 2>&1 && gst-inspect-1.0 nvvidconv >/dev/null 2>&1
}

install_plugins() {
    local command current hook
    for command in apt-get apt-cache dpkg-query dpkg-deb stat flock awk; do
        command -v "$command" >/dev/null || die "Missing command: $command"
    done
    install -d -m 0700 "$worker_dir"
    exec 9>"$worker_dir/gst.lock"
    flock -w "$wait_timeout" 9 || die 'Another sgMIX plugin task is still running'
    wait_for_apt
    sdk_state || { echo 'SDK removed; skipping plugin installation.'; return 0; }
    [[ $(dpkg --print-architecture) == arm64 ]] || die 'SDK requires an arm64 Jetson device'
    gst_tmp=$(mktemp -d /tmp/sgmix-gst.XXXXXXXX)
    snapshot > "$gst_tmp/installed"; load_snapshot "$gst_tmp/installed"
    core=${installed[nvidia-l4t-core:arm64]-}
    [[ $core =~ ^([0-9]+\.[0-9]+)\.[0-9]+-[A-Za-z0-9.+~]+$ ]] || die 'Cannot determine the complete nvidia-l4t-core version'
    [[ -z $(dpkg --audit) ]] || die 'Unfinished dpkg operations; refusing plugin installation'
    current=${installed["$plugin:arm64"]-}
    [[ -z $current || $current == "$core" ]] || die "Installed plugin is $current, but core is $core"
    echo "Installed BSP: $core; kernel: $(uname -r)"
    if [[ $current == "$core" ]] && plugins_ready; then echo 'NVIDIA plugins already work; nothing to install.'; return 0; fi
    [[ ${selections["$plugin:arm64"]-} != hold ]] || die 'The plugin package is held; refusing to modify it'
    apt-get -o APT::Update::Error-Mode=any update
    apt-cache madison "$plugin" > "$gst_tmp/versions"
    awk -F '|' -v version="$core" '{gsub(/^[ \t]+|[ \t]+$/, "", $2); if ($2==version) found=1} END {exit !found}' "$gst_tmp/versions" || die "No $plugin=$core in the configured repositories; no other version will be substituted"
    printf '%s\n' "$core" > "$gst_tmp/core"
    printf 'Package: nvidia-l4t-*\nPin: version %s\nPin-Priority: 990\n\nPackage: nvidia-l4t-*\nPin: version *\nPin-Priority: -1\n\n' "$core" > "$gst_tmp/preferences"
    [[ ! -f /etc/apt/preferences ]] || cat /etc/apt/preferences >> "$gst_tmp/preferences"
    local -a apt=(apt-get --no-remove --no-install-recommends --reinstall
        -o "Dir::Etc::preferences=$gst_tmp/preferences" -o APT::Get::AutomaticRemove=false
        -o DPkg::Lock::Timeout=60 install "$plugin=$core")
    command -v gst-inspect-1.0 >/dev/null || apt+=(gstreamer1.0-tools)
    apt-get --simulate "${apt[@]:1}" > "$gst_tmp/plan"
    cat "$gst_tmp/plan"; approve_plan "$gst_tmp"
    verify_state "$gst_tmp"
    printf -v hook '/bin/bash %q --apt-hook %q' "$(readlink -f "${BASH_SOURCE[0]}")" "$gst_tmp"
    apt-get -y -o "DPkg::Pre-Install-Pkgs::=$hook" "${apt[@]:1}"
    verify_state "$gst_tmp"
    [[ $(dpkg-query -W -f='${db:Status-Status}|${Version}' "$plugin") == "installed|$core" ]] && plugins_ready || die 'Plugin installation completed, but plugin loading failed'
    echo 'NVIDIA plugins ready: nvarguscamerasrc, nvvidconv'
}

main() {
    case "${1:-}" in
        --help|-h) echo 'Usage: sgmix-install-gst.sh [--schedule]'; return 0;;
        --apt-hook) [[ $# == 2 ]] || die 'Invalid APT hook'; archive_guard "$2"; return 0;;
    esac
    (( EUID == 0 )) || die 'Run this script with sudo'
    case "$#:${1:-}" in
        0:) install_plugins;;
        1:--schedule) schedule;;
        *) die "Unknown arguments: $*";;
    esac
}

if [[ ${BASH_SOURCE[0]} == "$0" ]]; then main "$@"; fi
