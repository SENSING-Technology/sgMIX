// 标定格式转换工具：客户交付的 txt（每台模组一份，文件名=模组序列号）→ json
// 用法: sdepth_calib_convert <input.txt> [-o output.json]
//   缺省输出路径: 与输入同目录同名、扩展名改 .json
// 注: 主程序 stereo_depth_app 可直接读 txt（--calib xxx.txt），本工具仅用于
//     需要 json 的场景（如旧版程序、人工核对）。

#include "calib_format.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
        std::cout << "Usage: " << argv[0] << " <input.txt> [-o output.json]" << std::endl;
        std::cout << "  Convert vendor stereo calibration txt to SDK json format." << std::endl;
        std::cout << "  Default output: <input>.json (same directory)" << std::endl;
        return (argc < 2) ? 1 : 0;
    }

    const std::string input = argv[1];
    std::string output;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            output = argv[++i];
        }
    }
    if (output.empty()) {
        output = input;
        const size_t dot = output.find_last_of('.');
        if (dot != std::string::npos) output = output.substr(0, dot);
        output += ".json";
    }

    try {
        StereoCalibData calib = ParseStereoCalibrationTxt(input);
        SaveStereoCalibrationJson(calib, output);
        std::cout << "[OK] " << input << " -> " << output << std::endl;
        std::cout << "  camera_model=" << calib.camera_model
                  << "  image=" << calib.image_size.width << "x" << calib.image_size.height
                  << "  baseline=" << cv::norm(calib.T) << " m"
                  << "  dist=" << calib.dist_left.cols << " coeffs" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
