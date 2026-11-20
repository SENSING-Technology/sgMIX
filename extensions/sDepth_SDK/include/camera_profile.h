#ifndef CAMERA_PROFILE_H
#define CAMERA_PROFILE_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

/**
 * 相机型号档案（config/cameras/<name>.json）
 *
 * 每款双目模组一份，描述 SIPL 采集所需的全部静态参数：
 *   - output:       "isp"（RAW 模组，走 ISP0 硬件 ISP → NV12）
 *                   "icp_yuv"（YUV 模组，ICP 直出 UYVY，不走 ISP）
 *   - backend:      采集后端（可选，默认 "sipl"）
 *                   "sipl" = SIPL 用户态（Thor 官方主推路径）
 *                   "v4l2" = 内核驱动 /dev/videoN 节点（SENSING Argus /
 *                            GMSL2x8 驱动包：左目 videoN，右目 videoN+1）
 *                   "auto" = 运行时自动探测（检测 /usr/lib/nvsipl_drv 下的
 *                            libnvuddf_<name>_*.so → sipl；否则 /dev/videoN
 *                            → v4l2。两个驱动包互斥，探测可靠）
 *   - sipl_json:    平台查询 json（相对 SDK 根目录，如 "sipl/s36.json"）
 *   - sipl_config:  SensorSystemConfig 名称（与驱动包 query json 内 name 一致）
 *   - capture_*:    传感器原生采集分辨率
 *   - calib_type:   期望的标定模型（"fisheye"/"pinhole"，仅作提示性校验）
 *   - ports:        GMSL 端口 → link 掩码（"cam0" → ["0x0001","0x0000"]，
 *                   两个 uint32 对应 Transport[0](AB) / Transport[1](CD)）
 */
struct CameraProfile {
    std::string name;
    std::string output;                    // "isp" | "icp_yuv"
    std::string backend = "sipl";          // "sipl" | "v4l2"
    std::string sipl_json;
    std::string sipl_config;
    int capture_width = 0;
    int capture_height = 0;
    int fps = 30;
    std::string calib_type;
    std::map<std::string, std::vector<uint32_t>> ports;
    // pipeline0/1 与标定左/右相反时为 true（模组接线决定；s56c 实测需 true）
    bool swap_lr = true;

    bool isYuv() const { return output == "icp_yuv"; }

    // 端口名（大小写不敏感，如 "cam2"）→ link 掩码；未配置抛 std::runtime_error
    std::vector<uint32_t> portMasks(const std::string& port) const;

    // 端口名 → 端口序号（"cam0"→0 ... "cam7"→7）；非法抛 std::runtime_error
    static int PortIndex(const std::string& port);
};

/**
 * 加载型号档案。name 支持：
 *   - 型号名（"s36" / "s56c" / "s56"，大小写不敏感）→ 依次探测
 *     config/cameras/<name>.json、../config/cameras/<name>.json、<exe>/../config/...
 *   - 或直接给 json 文件路径
 * 失败抛 std::runtime_error。
 */
CameraProfile LoadCameraProfile(const std::string& name_or_path);

#endif // CAMERA_PROFILE_H
