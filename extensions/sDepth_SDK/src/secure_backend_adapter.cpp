/*
 * secure_backend_adapter.cpp —— 推理后端适配层（开放源码）
 *
 * 将闭源库 libsdepth.so 的 C 接口（sdepth_api.h）适配为
 * 流水线使用的 IStereoDepthBackend 抽象接口。模型解密、license 验签、
 * TensorRT 推理全部发生在闭源库内部，本文件只做接口转换。
 */
#include "stereo_backend.h"
#include "sdepth_api.h"

#include <iostream>
#include <memory>
#include <string>

namespace {

class SecureDepthBackend final : public IStereoDepthBackend {
public:
    ~SecureDepthBackend() override {
        if (handle_) sdepth_backend_destroy(handle_);
    }

    bool init(const std::string& model_path_or_config,
              const std::string& license_path = "",
              const std::string& pubkey_path = "") override {
        handle_ = sdepth_backend_create(
            model_path_or_config.c_str(),
            license_path.empty() ? nullptr : license_path.c_str(),
            pubkey_path.empty() ? nullptr : pubkey_path.c_str());
        if (!handle_) {
            std::cerr << "[SecureBackend] " << sdepth_last_error() << std::endl;
            return false;
        }
        input_size_ = cv::Size(sdepth_backend_input_width(handle_),
                               sdepth_backend_input_height(handle_));
        return true;
    }

    bool infer(const cv::Mat& left_rect, const cv::Mat& right_rect, cv::Mat& disparity_out) override {
        if (!handle_) return false;
        if (sdepth_backend_infer(handle_, &left_rect, &right_rect, &disparity_out) != 0) {
            std::cerr << "[SecureBackend] " << sdepth_last_error() << std::endl;
            return false;
        }
        return true;
    }

    cv::Size getInputSize() const override { return input_size_; }
    std::string name() const override { return "SDepthSecure"; }

private:
    SDepthBackend* handle_ = nullptr;
    cv::Size input_size_{0, 0};
};

} // namespace

std::unique_ptr<IStereoDepthBackend> createBackend(const std::string& model_name_or_path,
                                                   const std::string& license_path,
                                                   const std::string& pubkey_path) {
    auto backend = std::make_unique<SecureDepthBackend>();
    if (!backend->init(model_name_or_path, license_path, pubkey_path)) {
        return nullptr;
    }
    return backend;
}
