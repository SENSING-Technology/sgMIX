#include "tensorrt_inference.h"
#include <chrono>
#include <iostream>
#include <thread>

InferenceThread::InferenceThread(SafeQueue<PreprocessedFrame>& input_queue,
                                 SafeQueue<InferenceResult>& output_queue,
                                 std::unique_ptr<IStereoDepthBackend> backend,
                                 bool enable_stats_output,
                                 bool enable_output)
    : input_queue_(input_queue),
      output_queue_(output_queue),
      backend_(std::move(backend)),
      enable_stats_output_(enable_stats_output),
      enable_output_(enable_output) {
    if (!backend_) {
        throw std::runtime_error("InferenceThread: backend is null.");
    }
}

InferenceThread::~InferenceThread() {
    stop();
}

void InferenceThread::start() {
    running_ = true;
    thread_ = std::thread(&InferenceThread::run, this);
}

void InferenceThread::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void InferenceThread::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void InferenceThread::run() {
    long long total_frames = 0;
    double total_infer_ms_sum = 0.0;
    long long window_frames = 0;
    double window_infer_ms_sum = 0.0;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_log_time = start_time;

    while (running_) {
        PreprocessedFrame p_frame;
        input_queue_.wait_and_pop(p_frame);

        PreprocessedFrame latest_frame = p_frame;
        while (input_queue_.try_pop(p_frame)) {
            if (!p_frame.left_rect.empty() && !p_frame.right_rect.empty()) {
                latest_frame = p_frame;
            }
        }

        // 检查帧是否为空
        if (latest_frame.left_rect.empty() || latest_frame.right_rect.empty()) {
            std::cerr << "Error: Empty rectified frames received for inference" << std::endl;
            continue;
        }

        // 推理优先使用预处理好的模型输入：
        // - 默认：使用预处理线程输出的 left_model/right_model（例如 ESS 的 RGB float32 [0,1]）
        // - 对于 RTIGEV：保持与 IGEV-YOLO8 Jetpack 示例一致，直接使用几何矫正后的 BGR 8UC3（不做额外归一化），
        //   归一化在后端或引擎内部完成。
        // - 对于 CoEx_Backend：使用原始矫正图像进行推理
        const std::string backend_name = backend_->name();
        const bool use_rect = (backend_name == "RTIGEV_Backend" || backend_name == "CoEx_Backend");

        const cv::Mat& net_left  = use_rect
                                   ? latest_frame.left_rect
                                   : (latest_frame.left_model.empty()  ? latest_frame.left_rect  : latest_frame.left_model);
        const cv::Mat& net_right = use_rect
                                   ? latest_frame.right_rect
                                   : (latest_frame.right_model.empty() ? latest_frame.right_rect : latest_frame.right_model);

        // 检查输入图像是否为空
        if (net_left.empty() || net_right.empty()) {
            std::cerr << "Warning: Empty input images for inference" << std::endl;
            continue;
        }

        auto t0 = std::chrono::high_resolution_clock::now();

        cv::Mat disparity_map;
        bool ok = backend_->infer(net_left, net_right, disparity_map);

        auto t_end = std::chrono::high_resolution_clock::now();
        double true_total_ms = std::chrono::duration<double, std::milli>(t_end - t0).count();

        if (!ok) {
            std::cerr << "Inference failed for frame " << latest_frame.frame_id << std::endl;
            continue;
        }

        if (disparity_map.empty()) {
            std::cerr << "Error: Empty disparity map for frame " << latest_frame.frame_id << std::endl;
            continue;
        }

        if (enable_stats_output_) {
            total_frames++;
            total_infer_ms_sum += true_total_ms;
            window_frames++;
            window_infer_ms_sum += true_total_ms;
            auto now = std::chrono::high_resolution_clock::now();
            double since_last_log = std::chrono::duration<double>(now - last_log_time).count();
            if (since_last_log >= 5.0 && window_frames > 0) {
                double fps = window_frames / since_last_log;
                double avg_infer_ms = window_infer_ms_sum / static_cast<double>(window_frames);
                std::cout << "[" << backend_->name() << "] FPS: " << fps << ", Time: " << avg_infer_ms << " ms (last 5s)" << std::endl;
                last_log_time = now;
                window_frames = 0;
                window_infer_ms_sum = 0.0;
            }
        }

        if (enable_output_) {
            InferenceResult out;
            out.frame_id = latest_frame.frame_id;
            out.left_image = latest_frame.left_rect.clone();
            out.disparity_map = std::move(disparity_map);
            out.inference_time_ms = true_total_ms;
            output_queue_.push(std::move(out));
        }

        if (target_frame_ms_ > 0 && true_total_ms < target_frame_ms_) {
            int sleep_ms = static_cast<int>(target_frame_ms_ - true_total_ms);
            if (sleep_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            }
        }
    }
}

void InferenceThread::setTargetFPS(int fps) {
    if (fps > 0) {
        target_fps_ = fps;
        target_frame_ms_ = 1000.0 / fps;
        std::cout << "Inference thread target FPS set to: " << fps << " (interval: " << target_frame_ms_ << " ms)" << std::endl;
    } else {
        target_fps_ = 0;
        target_frame_ms_ = 0.0;
        std::cout << "Inference thread FPS control disabled" << std::endl;
    }
}
