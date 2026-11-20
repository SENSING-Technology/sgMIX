#include "stereo_camera.h"
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>

#ifdef HAVE_SIPL
#include "sipl_camera.h"
#endif

V4L2Camera::V4L2Camera(const std::string& device_path, bool force_uyvy)
    : device_path_(device_path), force_uyvy_(force_uyvy) {}

V4L2Camera::~V4L2Camera() {
    close();
}

bool V4L2Camera::open(int width, int height, int fps) {
    fd_ = ::open(device_path_.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (fd_ < 0) {
        std::cerr << "Error: Could not open device " << device_path_ << std::endl;
        return false;
    }
    
    width_ = width;
    height_ = height;
    
    if (!setFormat(width, height)) {
        std::cerr << "Error: Could not set format for " << device_path_ << std::endl;
        close();
        return false;
    }
    
    if (fps > 0 && !setFps(fps)) {
        std::cerr << "Warning: Could not set fps for " << device_path_ << std::endl;
    }
    
    if (!initMmap()) {
        std::cerr << "Error: Could not initialize mmap for " << device_path_ << std::endl;
        close();
        return false;
    }
    
    if (!startStreaming()) {
        std::cerr << "Error: Could not start streaming for " << device_path_ << std::endl;
        close();
        return false;
    }
    
    return true;
}

void V4L2Camera::close() {
    if (fd_ >= 0) {
        stopStreaming();
        
        if (buffer_starts_) {
            for (unsigned int i = 0; i < buffer_count_; ++i) {
                if (buffer_starts_[i]) {
                    munmap(buffer_starts_[i], buffers_[i].length);
                }
            }
            delete[] buffer_starts_;
            buffer_starts_ = nullptr;
        }
        
        if (buffers_) {
            delete[] buffers_;
            buffers_ = nullptr;
        }
        
        ::close(fd_);
        fd_ = -1;
    }
}

bool V4L2Camera::setFormat(int width, int height) {
    // First, enumerate supported formats to find what's available
    struct v4l2_fmtdesc fmtdesc = {};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bool supports_yuyv = false;
    bool supports_uyvy = false;

    while (ioctl(fd_, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_YUYV) supports_yuyv = true;
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_UYVY) supports_uyvy = true;
        fmtdesc.index++;
    }

    // Prefer YUYV, fall back to UYVY
    __u32 requested_fmt = V4L2_PIX_FMT_YUYV;
    if (!supports_yuyv && supports_uyvy) {
        requested_fmt = V4L2_PIX_FMT_UYVY;
    }

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = requested_fmt;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
        std::cerr << "Error: VIDIOC_S_FMT failed" << std::endl;
        return false;
    }

    // Check what the driver actually set (it may negotiate a different format)
    pixel_format_ = fmt.fmt.pix.pixelformat;
    width_ = fmt.fmt.pix.width;
    height_ = fmt.fmt.pix.height;
    bytes_per_line_ = fmt.fmt.pix.bytesperline;

    std::cout << "[V4L2Camera] " << device_path_
              << " format: " << ((char*)&pixel_format_)[0]
              << ((char*)&pixel_format_)[1]
              << ((char*)&pixel_format_)[2]
              << ((char*)&pixel_format_)[3]
              << " (" << width_ << "x" << height_ << ")" << std::endl;

    if (pixel_format_ != V4L2_PIX_FMT_YUYV && pixel_format_ != V4L2_PIX_FMT_UYVY) {
        std::cerr << "[V4L2Camera] Unsupported pixel format: "
                  << ((char*)&pixel_format_)[0]
                  << ((char*)&pixel_format_)[1]
                  << ((char*)&pixel_format_)[2]
                  << ((char*)&pixel_format_)[3] << std::endl;
        return false;
    }

    return true;
}

bool V4L2Camera::setFps(int fps) {
    struct v4l2_streamparm streamparm = {};
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = fps;
    
    if (ioctl(fd_, VIDIOC_S_PARM, &streamparm) < 0) {
        return false;
    }
    return true;
}

bool V4L2Camera::initMmap() {
    struct v4l2_requestbuffers req = {};
    req.count = buffer_count_;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        std::cerr << "Error: VIDIOC_REQBUFS failed" << std::endl;
        return false;
    }
    
    buffers_ = new v4l2_buffer[buffer_count_];
    buffer_starts_ = new void*[buffer_count_];
    
    for (unsigned int i = 0; i < buffer_count_; ++i) {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
            std::cerr << "Error: VIDIOC_QUERYBUF failed" << std::endl;
            return false;
        }
        
        buffers_[i] = buf;
        buffer_starts_[i] = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        
        if (buffer_starts_[i] == MAP_FAILED) {
            std::cerr << "Error: mmap failed" << std::endl;
            return false;
        }
        
        if (ioctl(fd_, VIDIOC_QBUF, &buffers_[i]) < 0) {
            std::cerr << "Error: VIDIOC_QBUF failed" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool V4L2Camera::startStreaming() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "Error: VIDIOC_STREAMON failed" << std::endl;
        return false;
    }
    return true;
}

bool V4L2Camera::stopStreaming() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd_, VIDIOC_STREAMOFF, &type);
    return true;
}

uint64_t V4L2Camera::getTimestamp() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

bool V4L2Camera::captureFrame(CameraFrame& frame) {
    // 先 poll 等帧（1s 超时），避免 O_NONBLOCK 下 DQBUF/EAGAIN 空转烧 CPU
    struct pollfd pfd = {};
    pfd.fd = fd_;
    pfd.events = POLLIN;
    int pret = poll(&pfd, 1, 1000);
    if (pret <= 0) {
        return false;  // 超时或出错，由上层继续重试
    }

    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
        return false;
    }

    // 优先用内核 SOF 时间戳（CLOCK_MONOTONIC，双目两节点同一时钟域，
    // 配对精度远高于用户态取帧时刻）；驱动不给时回退系统时钟
    if ((buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC) &&
        (buf.timestamp.tv_sec || buf.timestamp.tv_usec)) {
        frame.timestamp_ns = static_cast<uint64_t>(buf.timestamp.tv_sec) * 1000000000ULL +
                             static_cast<uint64_t>(buf.timestamp.tv_usec) * 1000ULL;
    } else {
        frame.timestamp_ns = getTimestamp();
    }
    
    cv::Mat yuv_frame(height_, width_, CV_8UC2, buffer_starts_[buf.index]);
    if (force_uyvy_ || pixel_format_ == V4L2_PIX_FMT_UYVY) {
        cv::cvtColor(yuv_frame, frame.image, cv::COLOR_YUV2BGR_UYVY);
    } else {
        cv::cvtColor(yuv_frame, frame.image, cv::COLOR_YUV2BGR_YUYV);
    }
    
    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
        return false;
    }
    
    return true;
}

// ============== Argus Camera Implementation ==============

ArgusCamera::ArgusCamera(const std::string& pipeline_prefix) : pipeline_prefix_(pipeline_prefix) {
    // 初始化 GStreamer
    static bool gst_initialized = false;
    if (!gst_initialized) {
        gst_init(nullptr, nullptr);
        gst_initialized = true;
    }
}

ArgusCamera::~ArgusCamera() {
    close();
}

bool ArgusCamera::open(int width, int height, int fps) {
    width_ = width;
    height_ = height;
    fps_ = fps;
    
    // 构建 GStreamer pipeline，使用 appsink 获取硬件时间戳
    std::string gst_pipeline_str = 
        pipeline_prefix_ + 
        " ! video/x-raw(memory:NVMM),width=" + std::to_string(width) + 
        ",height=" + std::to_string(height) + 
        ",framerate=" + std::to_string(fps) + "/1" +
        " ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw,format=BGR ! appsink name=sink sync=false max-buffers=4 drop=true";
    
    std::cout << "[ArgusCamera] Opening pipeline: " << gst_pipeline_str << std::endl;
    
    // 解析 pipeline
    GError* error = nullptr;
    pipeline_ = gst_parse_launch(gst_pipeline_str.c_str(), &error);
    if (error != nullptr) {
        std::cerr << "[ArgusCamera] Failed to parse pipeline: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    
    if (!pipeline_) {
        std::cerr << "[ArgusCamera] Failed to create pipeline!" << std::endl;
        return false;
    }
    
    // 获取 appsink 元素
    appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");
    if (!appsink_) {
        std::cerr << "[ArgusCamera] Failed to get appsink element!" << std::endl;
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        return false;
    }
    
    // 设置 appsink 属性
    g_object_set(appsink_, 
                 "emit-signals", TRUE,
                 "sync", FALSE,
                 "max-buffers", 4,
                 "drop", TRUE,
                 nullptr);
    
    // 启动 pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[ArgusCamera] Failed to start pipeline!" << std::endl;
        gst_object_unref(appsink_);
        gst_object_unref(pipeline_);
        appsink_ = nullptr;
        pipeline_ = nullptr;
        return false;
    }
    
    // 等待相机稳定
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::cout << "[ArgusCamera] opened: " << pipeline_prefix_ << " (" << width_ << "x" << height_ << "@" << fps << "fps) with hardware timestamp support" << std::endl;
    return true;
}

void ArgusCamera::close() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        if (appsink_) {
            gst_object_unref(appsink_);
            appsink_ = nullptr;
        }
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}

bool ArgusCamera::captureFrame(CameraFrame& frame) {
    if (!pipeline_ || !appsink_) {
        return false;
    }
    
    // Try to pull sample with timeout (1 second)
    // First check if sample is available
    guint64 timeout_ns = 1'000'000'000ULL; // 1 second
    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink_), timeout_ns);
    if (!sample) {
        static int timeout_count = 0;
        if (timeout_count++ % 30 == 0) {
            std::cerr << "[ArgusCamera] WARNING: pull_sample timeout (" << pipeline_prefix_ << ")" << std::endl;
        }
        return false;
    }
    
    // 获取 buffer 和硬件时间戳 (PTS)
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        gst_sample_unref(sample);
        return false;
    }
    
    // 获取硬件时间戳 (GST_BUFFER_PTS 包含硬件时间戳，单位：纳秒)
    GstClockTime pts = GST_BUFFER_PTS(buffer);
    if (pts != GST_CLOCK_TIME_NONE) {
        frame.timestamp_ns = static_cast<uint64_t>(pts);
    } else {
        // 如果 PTS 不可用，使用系统时间作为回退
        auto now = std::chrono::high_resolution_clock::now();
        frame.timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        std::cerr << "[ArgusCamera] WARNING: Hardware timestamp NOT available, using system time (" << pipeline_prefix_ << ")" << std::endl;
    }
    
    // 获取视频帧数据
    GstCaps* caps = gst_sample_get_caps(sample);
    GstStructure* structure = gst_caps_get_structure(caps, 0);
    
    int frame_width, frame_height;
    gst_structure_get_int(structure, "width", &frame_width);
    gst_structure_get_int(structure, "height", &frame_height);
    
    // 映射 buffer 数据
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return false;
    }
    
    // 转换为 OpenCV Mat (BGR 格式)
    frame.image = cv::Mat(frame_height, frame_width, CV_8UC3, map.data).clone();
    
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    
    return true;
}

CaptureThread::CaptureThread(SafeQueue<StereoFrame>& output_queue, const std::string& left_cam_id, const std::string& right_cam_id, int width, int height, int fps, bool force_uyvy)
    : output_queue_(output_queue), left_cam_id_(left_cam_id), right_cam_id_(right_cam_id), frame_width_(width), frame_height_(height), cam_fps_(fps), force_uyvy_(force_uyvy) {}

CaptureThread::~CaptureThread() {
    stop();
}

void CaptureThread::start() {
    running_ = true;
    thread_ = std::thread(&CaptureThread::run, this);
}

void CaptureThread::stop() {
    running_ = false;
    
    if (left_thread_.joinable()) {
        left_thread_.join();
    }
    if (right_thread_.joinable()) {
        right_thread_.join();
    }
    if (thread_.joinable()) {
        thread_.join();
    }
    
    if (left_cam_) {
        delete left_cam_;
        left_cam_ = nullptr;
    }
    if (right_cam_) {
        delete right_cam_;
        right_cam_ = nullptr;
    }

#ifdef HAVE_SIPL
    // SIPL Manager 单例统一停止（两个 SiplCamera 适配器共享它）
    SiplCameraManager::getInstance().stop();
#endif
}

void CaptureThread::captureLeft() {
    int left_frame_id = 0;
    while (running_) {
        if (!left_cam_ || !left_cam_->isOpened()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        CameraFrame frame;
        if (left_cam_->captureFrame(frame)) {
            frame.frame_id = left_frame_id++;
            
            std::lock_guard<std::mutex> lock(left_mutex_);
            if (left_frames_.size() >= MAX_BUFFER_SIZE) {
                left_frames_.pop();
            }
            left_frames_.push(frame);
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void CaptureThread::captureRight() {
    int right_frame_id = 0;
    while (running_) {
        if (!right_cam_ || !right_cam_->isOpened()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        CameraFrame frame;
        if (right_cam_->captureFrame(frame)) {
            frame.frame_id = right_frame_id++;
            
            std::lock_guard<std::mutex> lock(right_mutex_);
            if (right_frames_.size() >= MAX_BUFFER_SIZE) {
                right_frames_.pop();
            }
            right_frames_.push(frame);
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}


void CaptureThread::matchAndPushFrames() {
    // For Argus: always use relative timestamps (different time bases per sensor)
    // For V4L2: use absolute timestamps (same system clock)
    bool use_relative_ts = use_argus_;
    if (use_relative_ts) {
        std::cout << "[Sync] Argus mode: using relative timestamp matching" << std::endl;
    } else {
        std::cout << "[Sync] V4L2 mode: PTS-window based pairing" << std::endl;
    }

    constexpr uint64_t MAX_PTS_DIFF_NS = 10'000'000ULL; // 10 ms

    uint64_t left_ts_offset = 0, right_ts_offset = 0;
    bool left_offset_set = false, right_offset_set = false;

    while (running_) {
        CameraFrame left_frame, right_frame;
        bool left_ok = false, right_ok = false;

        {
            std::lock_guard<std::mutex> lock(left_mutex_);
            left_ok = !left_frames_.empty();
        }
        {
            std::lock_guard<std::mutex> lock(right_mutex_);
            right_ok = !right_frames_.empty();
        }

        if (!left_ok || !right_ok) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(left_mutex_);
            left_frame = left_frames_.front();
        }
        {
            std::lock_guard<std::mutex> lock(right_mutex_);
            right_frame = right_frames_.front();
        }

        // Record first timestamp as offset for relative matching
        if (!left_offset_set) {
            left_ts_offset = left_frame.timestamp_ns;
            left_offset_set = true;
        }
        if (!right_offset_set) {
            right_ts_offset = right_frame.timestamp_ns;
            right_offset_set = true;
        }

        uint64_t pts_diff;
        if (use_relative_ts) {
            uint64_t left_relative = left_frame.timestamp_ns - left_ts_offset;
            uint64_t right_relative = right_frame.timestamp_ns - right_ts_offset;
            pts_diff = (left_relative > right_relative)
                ? (left_relative - right_relative)
                : (right_relative - left_relative);
        } else {
            pts_diff = (left_frame.timestamp_ns > right_frame.timestamp_ns)
                ? (left_frame.timestamp_ns - right_frame.timestamp_ns)
                : (right_frame.timestamp_ns - left_frame.timestamp_ns);
        }

        if (pts_diff > MAX_PTS_DIFF_NS) {
            if (use_relative_ts) {
                uint64_t left_relative = left_frame.timestamp_ns - left_ts_offset;
                uint64_t right_relative = right_frame.timestamp_ns - right_ts_offset;
                if (left_relative < right_relative) {
                    std::lock_guard<std::mutex> lock(left_mutex_);
                    left_frames_.pop();
                } else {
                    std::lock_guard<std::mutex> lock(right_mutex_);
                    right_frames_.pop();
                }
            } else {
                if (left_frame.timestamp_ns < right_frame.timestamp_ns) {
                    std::lock_guard<std::mutex> lock(left_mutex_);
                    left_frames_.pop();
                } else {
                    std::lock_guard<std::mutex> lock(right_mutex_);
                    right_frames_.pop();
                }
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(left_mutex_);
            left_frames_.pop();
        }
        {
            std::lock_guard<std::mutex> lock(right_mutex_);
            right_frames_.pop();
        }

        StereoFrame stereo;
        stereo.left  = left_frame.image.clone();
        stereo.right = right_frame.image.clone();
        stereo.frame_id = frame_count_++;
        stereo.left_timestamp_ns  = left_frame.timestamp_ns;
        stereo.right_timestamp_ns = right_frame.timestamp_ns;

        output_queue_.push(std::move(stereo));
    }
}

void CaptureThread::run() {
    std::cout << "[CaptureThread] Initializing cameras..." << std::endl;
    
    running_ = true;
    camera_initialized_ = false;
    
    bool use_argus = (left_cam_id_.find("nvarguscamerasrc") != std::string::npos) ||
                     (right_cam_id_.find("nvarguscamerasrc") != std::string::npos);
    use_argus_ = use_argus;

#ifdef HAVE_SIPL
    // SIPL: "sipl[:json:cfg:mask0,mask1]"（Thor 上 Argus 已死，官方主推用户态 SIPL）
    bool use_sipl = (left_cam_id_.rfind("sipl", 0) == 0) ||
                    (right_cam_id_.rfind("sipl", 0) == 0);
    use_sipl_ = use_sipl;
#else
    use_sipl_ = false;
#endif

    const char* backend = "V4L2";
    if (use_argus) backend = "Argus";
#ifdef HAVE_SIPL
    else if (use_sipl_) backend = "SIPL";
#endif
    std::cout << "[CaptureThread] Using " << backend << " backend" << std::endl;

#ifdef HAVE_SIPL
    if (use_sipl_) {
        // S56C 双目 = 单 GMSL module 同 link 的 VC0/VC1，
        // 由 SiplCameraManager 单例统一 Init/Start（sensor 0=左, 1=右）
        SiplCamParams sparams = SiplCamParams::parse(left_cam_id_);
        left_cam_ = new SiplCamera(0, sparams);
        right_cam_ = new SiplCamera(1, sparams);
    } else
#endif
    if (use_argus) {
        left_cam_ = new ArgusCamera(left_cam_id_);
        right_cam_ = new ArgusCamera(right_cam_id_);
    } else {
        left_cam_ = new V4L2Camera(left_cam_id_, force_uyvy_);
        right_cam_ = new V4L2Camera(right_cam_id_, force_uyvy_);
    }
    
    if (!left_cam_->open(frame_width_, frame_height_, cam_fps_)) {
        std::cerr << "[CaptureThread] Error: Could not open left camera! " << left_cam_id_ << std::endl;
        running_ = false;
        camera_initialized_ = false;
        return;
    }
    
    // 增加延迟，避免两个相机同时打开时的资源竞争
    if (use_argus) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    if (!right_cam_->open(frame_width_, frame_height_, cam_fps_)) {
        std::cerr << "[CaptureThread] Error: Could not open right camera! " << right_cam_id_ << std::endl;
        running_ = false;
        camera_initialized_ = false;
        return;
    }
    
    pthread_t native_handle = pthread_self();
    struct sched_param param;
    param.sched_priority = 50;
    pthread_setschedparam(native_handle, SCHED_FIFO, &param);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    left_thread_ = std::thread(&CaptureThread::captureLeft, this);
    right_thread_ = std::thread(&CaptureThread::captureRight, this);
    
    pthread_t left_handle = left_thread_.native_handle();
    pthread_t right_handle = right_thread_.native_handle();
    pthread_setschedparam(left_handle, SCHED_FIFO, &param);
    pthread_setschedparam(right_handle, SCHED_FIFO, &param);
    
    camera_initialized_ = true;
    std::cout << "[CaptureThread] Cameras initialized successfully" << std::endl;
    
    matchAndPushFrames();
}
