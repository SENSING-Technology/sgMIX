#include "camera_profile.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#else
#include "json.hpp"
#endif

namespace {

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

// 型号名 → 档案文件路径（按常见运行目录依次探测）
std::string LocateProfileFile(const std::string& name_or_path) {
    const std::string lower = Lower(name_or_path);
    const bool looks_like_name =
        lower.find('/') == std::string::npos && lower.find(".json") == std::string::npos;

    if (looks_like_name) {
        static const char* kDirs[] = {
            "config/cameras/",       // 工程根目录运行
            "../config/cameras/",    // encrypt_output/ 目录运行
        };
        for (const char* d : kDirs) {
            const std::string p = std::string(d) + lower + ".json";
            std::ifstream f(p);
            if (f.good()) return p;
        }
        throw std::runtime_error("未找到相机型号档案: " + lower +
                                 "（探测 config/cameras/" + lower + ".json；"
                                 "请确认在 SDK 根目录或 encrypt_output 下运行，"
                                 "或直接传档案 json 路径）");
    }

    std::ifstream f(name_or_path);
    if (!f.good()) {
        throw std::runtime_error("未找到相机型号档案: " + name_or_path);
    }
    return name_or_path;
}

uint32_t ParseMask(const nlohmann::json& v, const std::string& what) {
    if (v.is_string()) {
        return static_cast<uint32_t>(std::stoul(v.get<std::string>(), nullptr, 0));
    }
    if (v.is_number()) {
        return v.get<uint32_t>();
    }
    throw std::runtime_error("端口掩码格式非法（" + what + "），期望 \"0x0001\" 或整数");
}

} // namespace

std::vector<uint32_t> CameraProfile::portMasks(const std::string& port) const {
    auto it = ports.find(Lower(port));
    if (it == ports.end()) {
        std::ostringstream oss;
        oss << "型号 " << name << " 未配置端口 '" << port << "'，可用端口:";
        for (const auto& kv : ports) oss << " " << kv.first;
        throw std::runtime_error(oss.str());
    }
    return it->second;
}

int CameraProfile::PortIndex(const std::string& port) {
    const std::string p = Lower(port);
    if (p.size() == 4 && p.rfind("cam", 0) == 0 && p[3] >= '0' && p[3] <= '7') {
        return p[3] - '0';
    }
    throw std::runtime_error("非法端口名: '" + port + "'（期望 cam0..cam7）");
}

CameraProfile LoadCameraProfile(const std::string& name_or_path) {
    const std::string path = LocateProfileFile(name_or_path);
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("无法打开相机型号档案: " + path);
    }
    // 第4参 ignore_comments=true：允许档案里写 // 与 /* */ 中文注释
    nlohmann::json j = nlohmann::json::parse(f, nullptr, true, true);

    CameraProfile p;
    p.name = j.value("name", "");
    p.output = j.value("output", "");
    p.backend = Lower(j.value("backend", std::string("sipl")));
    p.sipl_json = j.value("sipl_json", "");
    p.sipl_config = j.value("sipl_config", "");
    p.capture_width = j.value("capture_width", 0);
    p.capture_height = j.value("capture_height", 0);
    p.fps = j.value("fps", 30);
    p.calib_type = j.value("calib_type", "");
    p.swap_lr = j.value("swap_lr", true);

    if (p.output != "isp" && p.output != "icp_yuv") {
        throw std::runtime_error("档案 " + path + " output 字段非法: '" + p.output +
                                 "'（期望 isp 或 icp_yuv）");
    }
    if (p.backend != "sipl" && p.backend != "v4l2" && p.backend != "argus" &&
        p.backend != "auto") {
        throw std::runtime_error("档案 " + path + " backend 字段非法: '" + p.backend +
                                 "'（期望 sipl / v4l2 / argus / auto）");
    }
    // auto 后端允许运行时探测（SIPL 驱动库 vs GMSL2x8 环境），sipl_json/sipl_config
    // 仍需保留以备选择 SIPL 时使用；argus/v4l2 不需要这两个字段
    if ((p.backend == "sipl" || p.backend == "auto") &&
        (p.sipl_json.empty() || p.sipl_config.empty())) {
        throw std::runtime_error("档案 " + path + " 缺少 sipl_json / sipl_config");
    }
    if (p.capture_width <= 0 || p.capture_height <= 0) {
        throw std::runtime_error("档案 " + path + " 采集分辨率非法");
    }

    if (j.contains("ports") && j["ports"].is_object()) {
        for (auto it = j["ports"].begin(); it != j["ports"].end(); ++it) {
            if (!it.value().is_array() || it.value().size() != 2) {
                throw std::runtime_error("档案 " + path + " 端口 " + it.key() +
                                         " 掩码需为 [mask0, mask1] 二元组");
            }
            p.ports[Lower(it.key())] = {ParseMask(it.value()[0], it.key()),
                                        ParseMask(it.value()[1], it.key())};
        }
    }
    if (p.ports.empty()) {
        throw std::runtime_error("档案 " + path + " 未配置任何端口掩码（ports）");
    }
    return p;
}
