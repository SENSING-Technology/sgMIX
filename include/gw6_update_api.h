#ifndef GW6_UPDATE_API_H
#define GW6_UPDATE_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* Preflight rejection: no ROM write was attempted. */
#define GW6_UPDATE_OTA_CHECK_FAILED (-10)

/* FSS 资源类型，取值与 iND880 SDK 的 Gw6FssResourceType_e 一致 */
#define GW6_FSS_TYPE_SYSTEM_INIT_JBF    2u
#define GW6_FSS_TYPE_ISP_JBF            4u
#define GW6_FSS_TYPE_LOADABLE_MODULE    6u
#define GW6_FSS_TYPE_USER_DATA          7u

/* 单文件模式下，文件名无法解析索引时使用的默认值 */
#define GW6_FSS_INDEX_ISP_JBF           128u
#define GW6_FSS_INDEX_LOADABLE_MODULE   640u
#define GW6_FSS_INDEX_USER_DATA         3584u

/* user_data 固定写入长度 */
#define GW6_USER_DATA_WRITE_SIZE        64u

#define GW6_OTA_TYPE_MASK_SYSTEM_INIT_JBF   (1u << GW6_FSS_TYPE_SYSTEM_INIT_JBF)
#define GW6_OTA_TYPE_MASK_ISP_JBF           (1u << GW6_FSS_TYPE_ISP_JBF)
#define GW6_OTA_TYPE_MASK_LOADABLE_MODULE   (1u << GW6_FSS_TYPE_LOADABLE_MODULE)
#define GW6_OTA_TYPE_MASK_USER_DATA         (1u << GW6_FSS_TYPE_USER_DATA)

#define GW6_OTA_TYPE_MASK_DEFAULT           (GW6_OTA_TYPE_MASK_SYSTEM_INIT_JBF \
                                             | GW6_OTA_TYPE_MASK_ISP_JBF \
                                             | GW6_OTA_TYPE_MASK_LOADABLE_MODULE \
                                             | GW6_OTA_TYPE_MASK_USER_DATA)

/* 0x02 GET_SYSTEM_STATE 返回的 systemState 取值（与 iND880 SDK 一致） */
#define GW6_SYSTEM_STATE_RECOVERY       0x00u
#define GW6_SYSTEM_STATE_INIT           0x01u
#define GW6_SYSTEM_STATE_ACTIVE         0x02u
#define GW6_SYSTEM_STATE_FAILSAFE       0x03u

/* Complete-ROM entry points always enforce the .rom file-type check.
 * Force skips configuration, Sensor and version checks only. */
int update_api(int i2cbus, char *filename);
int update_api_with_force(int i2cbus, char *filename, int force);
int get_prog_backup_state(int i2cbus);
int read_temperature(int i2cbus);
int update_file_isp(int i2cbus, char *filename);
int update_file_loadable_module(int i2cbus, char *filename);
int update_file_version(int i2cbus, char *filename);

/* 0x02 GET_SYSTEM_STATE：设备探活，ACTIVE/RECOVERY 视为可升级 */
int gw6_ota_probe(int i2cbus);

/* 解析完整 ROM，按资源类型批量写入（0x05），写完复位并校验版本。
 * force=0 时先做与 -f 相同的 OTA 预检（.rom 文件类型 + 配置/Sensor/版本），
 * force=1 跳过配置/Sensor/版本检查，但仍校验 .rom 文件类型。 */
int update_rom_auto(int i2cbus, const char *rom_path, uint32_t type_mask, int force);
/* Returns an allocated UTF-8 string on success, NULL on failure. */
int parse_get_firmwareverison(int i2cbus, char **out_str);
void parse_free_firmwareverison(char *ptr);
/* 升级失败后的收尾助手:静置等待设备收尾 + Info 探测,返回 0=已恢复,-1=疑似挂死 */
int settle_and_probe_device(int i2cbus);

#ifdef __cplusplus
}
#endif // __cplusplus   
#endif // GW6_UPDATE_API_H
