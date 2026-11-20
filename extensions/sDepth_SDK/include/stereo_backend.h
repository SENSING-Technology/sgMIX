#ifndef STEREO_BACKEND_H
#define STEREO_BACKEND_H

#include <opencv2/opencv.hpp>
#include <memory>
#include <string>

/**
 * 双目深度推理后端抽象接口。
 * 不同模型（TensorRT、ONNX、自定义等）实现此接口后即可接入通用流水线。
 */
class IStereoDepthBackend {
public:
    virtual ~IStereoDepthBackend() = default;

    /** 从模型路径或配置初始化，返回是否成功 */
    virtual bool init(const std::string& model_path_or_config,
                      const std::string& license_path = "",
                      const std::string& pubkey_path = "") = 0;

    /**
     * 执行双目推理。
     * @param left_rect  校正后的左图
     * @param right_rect 校正后的右图
     * @param disparity_out 输出的视差图 (CV_32FC1)，由实现方分配并写入
     * @return 是否成功
     */
    virtual bool infer(const cv::Mat& left_rect, const cv::Mat& right_rect, cv::Mat& disparity_out) = 0;

    /** 模型期望的输入尺寸 (width, height)，用于流水线中 resize 或校验 */
    virtual cv::Size getInputSize() const = 0;

    /** 后端名称，用于日志与区分模型 */
    virtual std::string name() const = 0;
};

/** 根据模型名或路径创建后端实例，当前支持 TensorRT (.trt)。后续可扩展为根据配置选择不同实现。 */
std::unique_ptr<IStereoDepthBackend> createBackend(const std::string& model_name_or_path,
                                                  const std::string& license_path = "",
                                                  const std::string& pubkey_path = "");

#endif // STEREO_BACKEND_H
