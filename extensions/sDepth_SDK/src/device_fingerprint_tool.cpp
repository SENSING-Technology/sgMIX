/*
 * device_fingerprint_tool.cpp —— 设备指纹生成工具（客户使用，开放源码）
 *
 * 采集本机硬件标识并用厂商公钥加密打包为 .finger 文件，发送给厂商
 * 用于生成绑定本机的加密模型。加密实现位于闭源库 libsdepth.so
 * （sdepth_generate_fingerprint）。
 */
#include "sdepth_api.h"

#include <iostream>
#include <string>

static void printUsage() {
    std::cout << "用法:\n";
    std::cout << "  ./device_finger --vendor-pub <vendor_public.pem> --out <fingerprint_file>\n\n";
    std::cout << "示例:\n";
    std::cout << "  ./device_finger --vendor-pub vendor_public.pem --out my_device.finger\n";
}

int main(int argc, char* argv[]) {
    std::string vendor_pub;
    std::string out_path;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--vendor-pub" && i + 1 < argc) {
            vendor_pub = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            out_path = argv[++i];
        }
    }

    if (vendor_pub.empty() || out_path.empty()) {
        printUsage();
        return 1;
    }

    if (sdepth_generate_fingerprint(vendor_pub.c_str(), out_path.c_str()) != 0) {
        std::cerr << "[ERR] " << sdepth_last_error() << "\n";
        return 1;
    }

    // 指纹文件已用厂商公钥加密，不含明文硬件信息
    std::cout << "Saved fingerprint file: " << out_path << "\n";
    return 0;
}
