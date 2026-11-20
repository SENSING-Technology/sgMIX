#ifndef TENSORRT_INFERENCE_H
#define TENSORRT_INFERENCE_H

#include "safe_queue.h"
#include "preprocess.h"
#include "stereo_backend.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <memory>

/** 推理结果，供可视化和后续流水线使用 */
struct InferenceResult {
    cv::Mat disparity_map;
    cv::Mat left_image;
    int frame_id;
    double inference_time_ms;
};

/**
 * 推理线程：从预处理队列取帧，调用后端推理，将结果放入输出队列。
 * 通过 IStereoDepthBackend 抽象支持多模型切换。
 */
class InferenceThread {
public:
    InferenceThread(SafeQueue<PreprocessedFrame>& input_queue,
                    SafeQueue<InferenceResult>& output_queue,
                    std::unique_ptr<IStereoDepthBackend> backend,
                    bool enable_stats_output,
                    bool enable_output = true);
    ~InferenceThread();

    void start();
    void stop();
    void join();
    void setTargetFPS(int fps);

private:
    void run();

    SafeQueue<PreprocessedFrame>& input_queue_;
    SafeQueue<InferenceResult>& output_queue_;
    std::unique_ptr<IStereoDepthBackend> backend_;
    bool enable_stats_output_ = false;
    bool enable_output_ = true;

    int target_fps_ = 0;
    double target_frame_ms_ = 0.0;

    std::thread thread_;
    std::atomic<bool> running_{false};
};

#endif // TENSORRT_INFERENCE_H
