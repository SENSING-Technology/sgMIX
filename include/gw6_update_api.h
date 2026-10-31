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

int update_api(int i2cbus, char *filename);
int get_prog_backup_state(int i2cbus);
int read_temperature(int i2cbus);
int update_file_isp(int i2cbus, char *filename);
int update_file_loadable_module(int i2cbus, char *filename);
int update_file_version(int i2cbus, char *filename);
int parse_get_firmwareverison(int i2cbus, char **out_str);
void parse_free_firmwareverison(char *ptr);
/* 升级失败后的收尾助手:静置等待设备收尾 + Info 探测,返回 0=已恢复,-1=疑似挂死 */
int settle_and_probe_device(int i2cbus);

#ifdef __cplusplus
}
#endif // __cplusplus   
#endif // GW6_UPDATE_API_H