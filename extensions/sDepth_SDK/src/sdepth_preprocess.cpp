#include "sdepth_preprocess.h"
#include "calib_format.h"
#include <iostream>
#include <fstream>

SDepthPreprocessThread::SDepthPreprocessThread(SafeQueue<StereoFrame>& input_queue,
                                                 SafeQueue<PreprocessedFrame>& output_queue,
                                                 const std::string& calib_file,
                                                 const cv::Size& original_size,
                                                 const cv::Size& target_size)
    : input_queue_(input_queue),
      output_queue_(output_queue),
      original_size_(original_size),
      target_size_(target_size),
      use_fisheye_(false),
      fisheye_balance_(0.0),
      fisheye_fov_scale_(1.0),
      auto_crop_black_border_(true),
      roi_initialized_(false) {
    load_calibration(calib_file);
    setup_stereo_rectification();
}

SDepthPreprocessThread::~SDepthPreprocessThread() {
    stop();
}

void SDepthPreprocessThread::load_calibration(const std::string& calib_file) {
    // 按扩展名自动识别 txt（客户交付格式）/ json（历史格式）
    StereoCalibData calib = LoadStereoCalibration(calib_file);

    use_fisheye_ = (calib.camera_model == "fisheye");
    std::cout << "[SDepthPreprocess] camera_model: " << calib.camera_model
              << " (from " << calib_file << ")" << std::endl;

    left_mtx_  = calib.K_left;
    right_mtx_ = calib.K_right;
    left_dist_  = calib.dist_left;
    right_dist_ = calib.dist_right;
    R_ = calib.R;
    T_ = calib.T;

    // 标定分辨率与采集分辨率不一致时缩放内参（fx/fy/cx/cy）
    const cv::Size calibration_size = calib.image_size;
    if (calibration_size.width > 0 && calibration_size.height > 0 &&
        original_size_.width > 0 && original_size_.height > 0 &&
        (calibration_size.width != original_size_.width ||
         calibration_size.height != original_size_.height)) {
        const double scale_x =
            static_cast<double>(original_size_.width) / calibration_size.width;
        const double scale_y =
            static_cast<double>(original_size_.height) / calibration_size.height;
        left_mtx_.at<double>(0, 0) *= scale_x;
        left_mtx_.at<double>(0, 2) *= scale_x;
        left_mtx_.at<double>(1, 1) *= scale_y;
        left_mtx_.at<double>(1, 2) *= scale_y;
        right_mtx_.at<double>(0, 0) *= scale_x;
        right_mtx_.at<double>(0, 2) *= scale_x;
        right_mtx_.at<double>(1, 1) *= scale_y;
        right_mtx_.at<double>(1, 2) *= scale_y;
        std::cout << "[SDepthPreprocess] Scaled intrinsics from "
                  << calibration_size.width << "x" << calibration_size.height
                  << " to " << original_size_.width << "x" << original_size_.height
                  << " (fx scale=" << scale_x << ", fy scale=" << scale_y << ")"
                  << std::endl;
    }

    fisheye_balance_ = calib.fisheye_balance;
    fisheye_fov_scale_ = calib.fisheye_fov_scale;
    auto_crop_black_border_ = calib.auto_crop_black_border;

    if (use_fisheye_) {
        std::cout << "[SDepthPreprocess] Using fisheye distortion model (balance="
                  << fisheye_balance_ << ", fov_scale=" << fisheye_fov_scale_
                  << ", auto_crop=" << auto_crop_black_border_ << ")" << std::endl;
    } else {
        std::cout << "[SDepthPreprocess] Using pinhole distortion model ("
                  << left_dist_.cols << " coeffs)" << std::endl;
    }
}

void SDepthPreprocessThread::setup_stereo_rectification() {
    if (use_fisheye_) {
        cv::fisheye::stereoRectify(left_mtx_, left_dist_,
                                   right_mtx_, right_dist_,
                                   original_size_, R_, T_,
                                   R1_, R2_, P1_, P2_, Q_,
                                   cv::fisheye::CALIB_ZERO_DISPARITY, target_size_,
                                   fisheye_balance_, fisheye_fov_scale_);
        cv::fisheye::initUndistortRectifyMap(left_mtx_, left_dist_, R1_, P1_,
                                             target_size_, CV_16SC2, left_map1_, left_map2_);
        cv::fisheye::initUndistortRectifyMap(right_mtx_, right_dist_, R2_, P2_,
                                             target_size_, CV_16SC2, right_map1_, right_map2_);
    } else {
        cv::stereoRectify(left_mtx_, left_dist_, right_mtx_, right_dist_,
                          original_size_, R_, T_,
                          R1_, R2_, P1_, P2_, Q_,
                          cv::CALIB_ZERO_DISPARITY, 0, target_size_);
        cv::initUndistortRectifyMap(left_mtx_, left_dist_, R1_, P1_,
                                    target_size_, CV_16SC2, left_map1_, left_map2_);
        cv::initUndistortRectifyMap(right_mtx_, right_dist_, R2_, P2_,
                                    target_size_, CV_16SC2, right_map1_, right_map2_);
    }

    baseline_     = cv::norm(T_);
    focal_length_ = P1_.at<double>(0, 0);

    std::cout << "[SDepthPreprocess] Rectification maps: " << target_size_
              << " baseline=" << baseline_ << " focal=" << focal_length_ << std::endl;
}

cv::Rect SDepthPreprocessThread::detectNonBlackRoi(const cv::Mat& bgr, int threshold) const {
    if (bgr.empty()) {
        return cv::Rect();
    }

    cv::Mat gray;
    if (bgr.channels() == 3) {
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = bgr;
    }

    cv::Mat mask;
    cv::threshold(gray, mask, threshold, 255, cv::THRESH_BINARY);
    cv::Rect roi = cv::boundingRect(mask);

    const int min_w = static_cast<int>(bgr.cols * 0.55);
    const int min_h = static_cast<int>(bgr.rows * 0.55);
    if (roi.width < min_w || roi.height < min_h) {
        const int margin_x = static_cast<int>(bgr.cols * 0.06);
        const int margin_y = static_cast<int>(bgr.rows * 0.06);
        roi = cv::Rect(margin_x, margin_y, bgr.cols - 2 * margin_x, bgr.rows - 2 * margin_y);
    }

    const int inset_x = static_cast<int>(roi.width * 0.01);
    const int inset_y = static_cast<int>(roi.height * 0.01);
    roi.x += inset_x;
    roi.y += inset_y;
    roi.width  = std::max(1, roi.width - 2 * inset_x);
    roi.height = std::max(1, roi.height - 2 * inset_y);

    roi &= cv::Rect(0, 0, bgr.cols, bgr.rows);
    return roi;
}

void SDepthPreprocessThread::cropBlackBorderAndResize(cv::Mat& img, const cv::Rect& roi) const {
    if (img.empty() || roi.width <= 0 || roi.height <= 0) {
        return;
    }
    if (roi.width >= img.cols && roi.height >= img.rows) {
        return;
    }

    const cv::Size original_size = img.size();
    cv::Mat cropped = img(roi).clone();
    cv::resize(cropped, img, original_size, 0, 0, cv::INTER_LINEAR);
}

void SDepthPreprocessThread::start() {
    running_ = true;
    thread_ = std::thread(&SDepthPreprocessThread::run, this);
}

void SDepthPreprocessThread::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void SDepthPreprocessThread::run() {
    while (running_) {
        StereoFrame raw_frame;
        input_queue_.wait_and_pop(raw_frame);

        if (raw_frame.left.empty() || raw_frame.right.empty()) continue;

        PreprocessedFrame processed_frame;
        processed_frame.frame_id = raw_frame.frame_id;

        cv::remap(raw_frame.left,  processed_frame.left_rect,  left_map1_,  left_map2_,  cv::INTER_LINEAR);
        cv::remap(raw_frame.right, processed_frame.right_rect, right_map1_, right_map2_, cv::INTER_LINEAR);

        if (use_fisheye_ && auto_crop_black_border_) {
            if (!roi_initialized_) {
                cv::Rect roi_l = detectNonBlackRoi(processed_frame.left_rect);
                cv::Rect roi_r = detectNonBlackRoi(processed_frame.right_rect);
                cached_roi_ = roi_l & roi_r;
                if (cached_roi_.width <= 0 || cached_roi_.height <= 0) {
                    cached_roi_ = roi_l.width > 0 && roi_l.height > 0 ? roi_l : roi_r;
                }
                roi_initialized_ = true;
                std::cout << "[SDepthPreprocess] Fisheye valid ROI: " << cached_roi_.width << "x"
                          << cached_roi_.height << " at (" << cached_roi_.x << "," << cached_roi_.y << ")"
                          << std::endl;
            }
            cropBlackBorderAndResize(processed_frame.left_rect, cached_roi_);
            cropBlackBorderAndResize(processed_frame.right_rect, cached_roi_);
        }

        cv::Mat left_rgb8, right_rgb8;
        cv::cvtColor(processed_frame.left_rect, left_rgb8, cv::COLOR_BGR2RGB);
        cv::cvtColor(processed_frame.right_rect, right_rgb8, cv::COLOR_BGR2RGB);
        left_rgb8.convertTo(processed_frame.left_model, CV_32F);
        right_rgb8.convertTo(processed_frame.right_model, CV_32F);

        output_queue_.push(processed_frame);
    }
}