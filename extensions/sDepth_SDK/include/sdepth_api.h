#ifndef SDEPTH_API_H
#define SDEPTH_API_H

/*
 * SDepth++ 双目深度 SDK —— 闭源推理库公共接口
 * libsdepth.so
 *
 * 本头文件是闭源库 libsdepth.so 对外的唯一接口，覆盖客户侧
 * 全部需求：
 *   1) 用厂商公钥生成本机设备指纹（发给厂商用于定制加密模型）
 *   2) 加载加密模型（.enc + .license）并进行实时双目深度推理
 *
 * 模型解密、license 验签、设备指纹比对、TensorRT 推理核心均封装在
 * libsdepth.so 内部，SDK 不包含其源码。
 *
 * 注意：
 * - 库为 aarch64 (Jetson AGX Thor / JetPack 7.2) 预编译，请勿改动
 *   lib/ 目录位置（可执行文件通过 $ORIGIN/../lib 相对路径查找）。
 * - 编译时需包含 OpenCV 头文件（板载 OpenCV 4.8，与库构建版本一致）。
 */

#include <opencv2/core.hpp>

#if defined(__GNUC__) || defined(__clang__)
#define SDEPTH_API __attribute__((visibility("default")))
#else
#define SDEPTH_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * 1. 设备指纹生成（客户使用厂商公钥调用）
 * ===================================================================== */

/*
 * 采集本机硬件标识（EMMC/CPU/GPU UUID 等），用厂商公钥加密打包为
 * 指纹文件。指纹文件不含明文硬件信息，发送给厂商即可。
 *
 * @param vendor_pub_pem_path  厂商公钥路径（vendor_public.pem）
 * @param out_finger_path      输出指纹文件路径（如 my_device.finger）
 * @return 0 成功；非 0 失败（错误信息见 sdepth_last_error）
 */
SDEPTH_API int sdepth_generate_fingerprint(const char* vendor_pub_pem_path,
                                                         const char* out_finger_path);

/* =====================================================================
 * 2. 加密模型实时推理
 * ===================================================================== */

/* 不透明推理后端句柄（内部持有 TensorRT 引擎与解密上下文） */
typedef struct SDepthBackend SDepthBackend;

/*
 * 创建推理后端：加载加密模型 -> license 验签 -> 本机设备指纹比对 ->
 * 解密模型 -> TensorRT 引擎反序列化。任一环节失败返回 NULL。
 *
 * @param model_enc_path        加密模型路径（.enc）
 * @param license_path          授权文件路径（.license）；传 NULL 时自动
 *                              使用 <model_enc_path 去掉扩展名>.license
 * @param vendor_pub_pem_path   厂商公钥路径；传 NULL 时使用当前目录下
 *                              的 vendor_public.pem
 * @return 成功返回句柄；失败返回 NULL（见 sdepth_last_error）
 */
SDEPTH_API SDepthBackend* sdepth_backend_create(const char* model_enc_path,
                                                                    const char* license_path,
                                                                    const char* vendor_pub_pem_path);

/* 模型输入宽度（像素）；句柄无效时返回 0 */
SDEPTH_API int sdepth_backend_input_width(const SDepthBackend* backend);

/* 模型输入高度（像素）；句柄无效时返回 0 */
SDEPTH_API int sdepth_backend_input_height(const SDepthBackend* backend);

/*
 * 单帧双目推理。
 *
 * @param left / right   校正后的左右目图像，支持两种格式：
 *                       - BGR 8UC3（任意尺寸，库内部缩放/转 RGB）
 *                       - RGB 32FC3（尺寸等于模型输入尺寸时零拷贝直通）
 * @param disparity_out  输出视差图（CV_32FC1，尺寸与输入图像一致，
 *                       已按缩放比例换算回原图尺度）
 * @return 0 成功；非 0 失败（见 sdepth_last_error）
 */
SDEPTH_API int sdepth_backend_infer(SDepthBackend* backend,
                                                  const cv::Mat* left,
                                                  const cv::Mat* right,
                                                  cv::Mat* disparity_out);

/* 销毁后端并释放全部资源 */
SDEPTH_API void sdepth_backend_destroy(SDepthBackend* backend);

/* =====================================================================
 * 3. 错误信息
 * ===================================================================== */

/* 最近一次失败的错误描述（线程局部），仅在同线程失败后读取有意义 */
SDEPTH_API const char* sdepth_last_error(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SDEPTH_API_H */
