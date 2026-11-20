#include "calib_format.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#else
#include "json.hpp"
#endif

namespace {

std::string Trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::string LowerExt(const std::string& path) {
    const size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext;
}

// ---------------- txt 解析 ----------------

// 取 key 对应的 double；缺失时报错（带文件名与 key）
double RequireKey(const std::map<std::string, double>& kv,
                  const std::string& key, const std::string& path) {
    auto it = kv.find(key);
    if (it == kv.end()) {
        throw std::runtime_error("标定文件 " + path + " 缺少字段: " + key);
    }
    return it->second;
}

bool HasKey(const std::map<std::string, double>& kv, const std::string& key) {
    return kv.find(key) != kv.end();
}

// 解析一只目：fx/fy/cx/cy + 畸变系数
// fisheye: (k1,k2,k3,k4)
// pinhole: (k1,k2,p1,p2,k3) + 可选 (k4,k5,k6) → OpenCV rational 8 系数顺序
void ParseOneEye(const std::map<std::string, double>& kv, const std::string& prefix,
                 bool fisheye, const std::string& path,
                 cv::Mat& K, cv::Mat& dist) {
    K = (cv::Mat_<double>(3, 3) <<
         RequireKey(kv, prefix + "_fx", path), 0.0, RequireKey(kv, prefix + "_cx", path),
         0.0, RequireKey(kv, prefix + "_fy", path), RequireKey(kv, prefix + "_cy", path),
         0.0, 0.0, 1.0);

    std::vector<double> d;
    if (fisheye) {
        for (const char* k : {"k1", "k2", "k3", "k4"}) {
            d.push_back(RequireKey(kv, prefix + "_" + k, path));
        }
    } else {
        d.push_back(RequireKey(kv, prefix + "_k1", path));
        d.push_back(RequireKey(kv, prefix + "_k2", path));
        d.push_back(HasKey(kv, prefix + "_p1") ? kv.at(prefix + "_p1") : 0.0);
        d.push_back(HasKey(kv, prefix + "_p2") ? kv.at(prefix + "_p2") : 0.0);
        d.push_back(HasKey(kv, prefix + "_k3") ? kv.at(prefix + "_k3") : 0.0);
        // rational 模型扩展项（S36 提供 k4~k6）
        if (HasKey(kv, prefix + "_k4") || HasKey(kv, prefix + "_k5") ||
            HasKey(kv, prefix + "_k6")) {
            d.push_back(HasKey(kv, prefix + "_k4") ? kv.at(prefix + "_k4") : 0.0);
            d.push_back(HasKey(kv, prefix + "_k5") ? kv.at(prefix + "_k5") : 0.0);
            d.push_back(HasKey(kv, prefix + "_k6") ? kv.at(prefix + "_k6") : 0.0);
        }
    }
    dist = cv::Mat(d).clone();  // Nx1 → 转 1xN 保持与 json 路径一致
    cv::transpose(dist, dist);
}

} // namespace

StereoCalibData ParseStereoCalibrationTxt(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("无法打开标定文件: " + path);
    }

    std::map<std::string, double> kv;
    std::string type;
    int img_w = 0, img_h = 0;

    std::string line;
    int lineno = 0;
    while (std::getline(f, line)) {
        ++lineno;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;  // 非 key=value 行（空行等）直接跳过
        const std::string key = Trim(line.substr(0, eq));
        const std::string val = Trim(line.substr(eq + 1));
        if (key.empty() || val.empty()) continue;

        if (key == "type") {
            type = val;
            continue;
        }
        // 数值行：value 可能带尾注（如 "2.6700 mm"、"2.6593 [offset: ...]"），
        // strtod 到第一个非数字字符为止，满足 key 白名单才保留。
        char* end = nullptr;
        const double v = std::strtod(val.c_str(), &end);
        if (end == val.c_str()) {
            continue;  // 非数值（如单位标注行），忽略
        }
        kv[key] = v;
    }

    if (type != "fisheye" && type != "pinhole") {
        throw std::runtime_error("标定文件 " + path + " type 字段非法: '" + type +
                                 "'（期望 fisheye 或 pinhole）");
    }
    img_w = static_cast<int>(RequireKey(kv, "imageWidth", path));
    img_h = static_cast<int>(RequireKey(kv, "imageHeight", path));

    StereoCalibData c;
    c.camera_model = type;
    c.image_size = cv::Size(img_w, img_h);

    const bool fisheye = (type == "fisheye");
    ParseOneEye(kv, "CAM0", fisheye, path, c.K_left, c.dist_left);
    ParseOneEye(kv, "CAM1", fisheye, path, c.K_right, c.dist_right);

    c.R = cv::Mat::eye(3, 3, CV_64F);
    for (int r = 0; r < 3; ++r) {
        for (int col = 0; col < 3; ++col) {
            const std::string key =
                "R[" + std::to_string(r) + "," + std::to_string(col) + "]";
            c.R.at<double>(r, col) = RequireKey(kv, key, path);
        }
    }
    c.T = cv::Mat(3, 1, CV_64F);
    for (int r = 0; r < 3; ++r) {
        const std::string key = "T[" + std::to_string(r) + ",0]";
        c.T.at<double>(r, 0) = RequireKey(kv, key, path);
    }
    // txt 的 T 单位为 mm，统一换算为 m，与历史 json 格式一致
    // （depth = focal * baseline / disp，baseline 以 m 计时深度单位才是 m）
    c.T /= 1000.0;
    return c;
}

// ---------------- json 解析（历史格式，逻辑自 sdepth_preprocess.cpp 迁入） ----------------

namespace {

cv::Mat JsonToMat(const nlohmann::json& j, int rows, int cols) {
    std::vector<double> flat;
    if (j.is_array()) {
        for (const auto& row : j) {
            if (row.is_array()) {
                for (const auto& val : row) flat.push_back(val.get<double>());
            } else {
                flat.push_back(row.get<double>());
            }
        }
    }
    if (static_cast<int>(flat.size()) != rows * cols) {
        throw std::runtime_error("json 标定矩阵尺寸不符: 期望 " + std::to_string(rows) + "x" +
                                 std::to_string(cols) + ", 实际 " +
                                 std::to_string(flat.size()) + " 元素");
    }
    return cv::Mat(rows, cols, CV_64F, flat.data()).clone();
}

cv::Mat JsonToDist(const nlohmann::json& j, const std::string& name) {
    std::vector<double> flat;
    if (j.is_array()) {
        for (const auto& row : j) {
            if (row.is_array()) {
                for (const auto& val : row) flat.push_back(val.get<double>());
            } else {
                flat.push_back(row.get<double>());
            }
        }
    }
    if (flat.empty()) {
        throw std::runtime_error("json 标定缺少畸变系数: " + name);
    }
    return cv::Mat(1, static_cast<int>(flat.size()), CV_64F, flat.data()).clone();
}

} // namespace

StereoCalibData ParseStereoCalibrationJson(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("无法打开标定文件: " + path);
    }
    nlohmann::json data = nlohmann::json::parse(f);

    StereoCalibData c;
    if (data.contains("camera_model") && data["camera_model"].is_string()) {
        c.camera_model = data["camera_model"].get<std::string>();
    }

    c.K_left  = JsonToMat(data["left_camera_matrix"], 3, 3);
    c.K_right = JsonToMat(data["right_camera_matrix"], 3, 3);
    c.R       = JsonToMat(data["rotation_matrix"], 3, 3);
    c.T       = JsonToMat(data["translation_vector"], 3, 1);
    c.dist_left  = JsonToDist(data["left_distortion_coefficients"], "left");
    c.dist_right = JsonToDist(data["right_distortion_coefficients"], "right");

    // camera_model 缺省时按畸变系数数量推断（4 → fisheye，与历史行为一致）
    if (c.camera_model.empty()) {
        c.camera_model = (c.dist_left.cols == 4) ? "fisheye" : "pinhole";
    }

    if (data.contains("calibration_size") && data["calibration_size"].is_array() &&
        data["calibration_size"].size() == 2) {
        c.image_size.width  = data["calibration_size"][0].get<int>();
        c.image_size.height = data["calibration_size"][1].get<int>();
    }
    if (data.contains("fisheye_balance")) {
        c.fisheye_balance = data["fisheye_balance"].get<double>();
    }
    if (data.contains("fisheye_fov_scale")) {
        c.fisheye_fov_scale = data["fisheye_fov_scale"].get<double>();
    }
    if (data.contains("auto_crop_black_border")) {
        c.auto_crop_black_border = data["auto_crop_black_border"].get<bool>();
    }
    return c;
}

StereoCalibData LoadStereoCalibration(const std::string& path) {
    if (LowerExt(path) == "txt") {
        return ParseStereoCalibrationTxt(path);
    }
    return ParseStereoCalibrationJson(path);
}

// ---------------- json 导出（转换工具用） ----------------

void SaveStereoCalibrationJson(const StereoCalibData& calib, const std::string& path) {
    nlohmann::json j;
    j["camera_model"] = calib.camera_model;
    j["calibration_size"] = {calib.image_size.width, calib.image_size.height};

    auto mat_to_json = [](const cv::Mat& m) {
        nlohmann::json out = nlohmann::json::array();
        for (int r = 0; r < m.rows; ++r) {
            nlohmann::json row = nlohmann::json::array();
            for (int col = 0; col < m.cols; ++col) row.push_back(m.at<double>(r, col));
            out.push_back(row);
        }
        return out;
    };
    auto dist_to_json = [](const cv::Mat& m) {
        nlohmann::json out = nlohmann::json::array();
        for (int c = 0; c < m.cols; ++c) out.push_back(m.at<double>(0, c));
        return out;
    };

    j["left_camera_matrix"]  = mat_to_json(calib.K_left);
    j["right_camera_matrix"] = mat_to_json(calib.K_right);
    j["rotation_matrix"]     = mat_to_json(calib.R);
    j["translation_vector"]  = mat_to_json(calib.T);
    j["left_distortion_coefficients"]  = dist_to_json(calib.dist_left);
    j["right_distortion_coefficients"] = dist_to_json(calib.dist_right);
    j["fisheye_balance"] = calib.fisheye_balance;
    j["fisheye_fov_scale"] = calib.fisheye_fov_scale;
    j["auto_crop_black_border"] = calib.auto_crop_black_border;

    std::ofstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("无法写出标定 json: " + path);
    }
    f << j.dump(4) << std::endl;
}
