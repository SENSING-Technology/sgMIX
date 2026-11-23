# 一.文档目的

本文档旨在为开发者提供森云相机控制及 ISP 功能 SDK 的使用说明。该 SDK 提供对相机模块的基础控制能力，包括图像翻转（flip）、镜像（mirror）、测试图案（test pattern）生成、相机信息读取、相机内参读取以及远程固件升级（OTA）功能。同时，为开发者提供图像采集、处理及优化的完整接口。

本文档面向软件开发者及系统集成工程师，旨在帮助用户快速了解 SDK 架构、正确安装与配置、快速集成相机功能，并通过示例代码掌握各类接口的调用方法，从而高效开发基于森云相机的应用程序。

# 二.适用范围

**V2.0 SDK主要适用于 NVIDIA Jetson 平台，支持以下环境：**

**操作系统**：JetPack 7.2.0

**硬件平台**：Jetson AGX Thor DK

**驱动框架**：v4l2、sipl

**相机设备**：本文覆盖 SG3S-ISX031C-GMSL2F-Hxxx、SG3S-ISX031C-GMSL2-Hxxx、SG3S11AFLK、SHF3L、M3A、SHF3H、SYF3L、SHW3G、SHW5G、SHF5G、S36、S36A、S56、S56C、S56D、SYF2G、SYW2G、SDT2BCO1 和 S56Y。各型号的功能支持情况见下方 V4L2 和 SIPL 表格。

# 三.SDK 功能概览

**SDK 提供的核心功能包括：**

1. 相机信息读取
   1. 相机基本信息：分辨率、帧率、图像格式。
   2. 镜头信息：视场角（FOV）、焦距、光圈、镜头型号。
   3. 相机标定数据：左右相机内参、畸变系数、双目外参（R/T）、重投影误差。
   4. IMU 标定数据：陀螺仪、加速度计参数、Bias、Scale、Misalignment、相机与 IMU 外参。
   5. 模组管理信息：SN 序列号。
2. 相机基本控制
   1. 图像翻转（Flip）
   2. 镜像（Mirror）
   3. 测试图案生成（Test Pattern）
3. 远程升级与管理
   1. OTA固件升级
4. 视频流控制
5. 畸变矫正
6. 帧信息解析

## v4l2

| <br />                                                                | **<font style="color:rgb(0, 0, 0);">读模组基本信息</font>** | **<font style="color:rgb(0, 0, 0);">读模组镜头信息</font>** | **<font style="color:rgb(0, 0, 0);">读模组SN码</font>** | **<font style="color:rgb(0, 0, 0);">读模组标定数据</font>** | **<font style="color:rgb(0, 0, 0);">镜像翻转控制</font>** | **<font style="color:rgb(0, 0, 0);">测试模式</font>** | **<font style="color:rgb(0, 0, 0);">OTA</font>** | **<font style="color:rgb(0, 0, 0);">视频流获取</font>** | **<font style="color:rgb(0, 0, 0);">帧信息解析</font>** | **<font style="color:rgb(0, 0, 0);">畸变矫正</font>** |
| --------------------------------------------------------------------- | :--------------------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | :-----------------------------------------------: | :----------------------------------------------: | :------------------------------------------------: | :------------------------------------------------: | :-----------------------------------------------: |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2F-Hxxx</font>** |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2-Hxxx</font>**  |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SG3S11AFLK</font>**               |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHW3G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHW5G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHF5G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S36</font>**                      |       **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S36A</font>**                     |       **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56</font>**                      |       **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56C</font>**                     |       **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56D</font>**                     |       **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYF2G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYW2G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SDT2BCO1</font>**                 |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       | |
| **<font style="color:rgb(0, 0, 0);">S56Y</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       | |
| **<font style="color:rgb(0, 0, 0);">SHF3L</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">M3A</font>**                      |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHF3H</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYF3L</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |

## sipl

| <br />                                                                | **<font style="color:rgb(0, 0, 0);">读模组基本信息</font>** | **<font style="color:rgb(0, 0, 0);">读模组镜头信息</font>** | **<font style="color:rgb(0, 0, 0);">读模组SN码</font>** | **<font style="color:rgb(0, 0, 0);">读模组标定数据</font>** | **<font style="color:rgb(0, 0, 0);">镜像翻转控制</font>** | **<font style="color:rgb(0, 0, 0);">测试模式</font>** | **<font style="color:rgb(0, 0, 0);">OTA</font>** | **<font style="color:rgb(0, 0, 0);">视频流获取</font>** | **<font style="color:rgb(0, 0, 0);">帧信息解析</font>** | **<font style="color:rgb(0, 0, 0);">畸变矫正</font>** |
| --------------------------------------------------------------------- | :--------------------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | :-----------------------------------------------: | :----------------------------------------------: | :------------------------------------------------: | :------------------------------------------------: | :-----------------------------------------------: |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2F-Hxxx</font>** |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SG3S-ISX031C-GMSL2-Hxxx</font>**  |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SG3S11AFLK</font>**               |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHW3G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHW5G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHF5G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S36</font>**                      |    **<font style="color:rgb(0, 0, 0);">●</font>**    |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S36A</font>**                     |    **<font style="color:rgb(0, 0, 0);">●</font>**    |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56</font>**                      |    **<font style="color:rgb(0, 0, 0);">●</font>**    |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56C</font>**                     |    **<font style="color:rgb(0, 0, 0);">●</font>**    |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">S56D</font>**                     |    **<font style="color:rgb(0, 0, 0);">●</font>**    |       **<font style="color:#1f2329;">●</font>**      |      **<font style="color:#1f2329;">●</font>**      |       **<font style="color:#1f2329;">●</font>**      |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYF2G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYW2G</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SDT2BCO1</font>**                 |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       | |
| **<font style="color:rgb(0, 0, 0);">S56Y</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |                        <br />                       |                       <br />                      |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |                       <br />                       | |
| **<font style="color:rgb(0, 0, 0);">SHF3L</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">M3A</font>**                      |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SHF3H</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |
| **<font style="color:rgb(0, 0, 0);">SYF3L</font>**                    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |    **<font style="color:rgb(0, 0, 0);">●</font>**    |    **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |  **<font style="color:rgb(0, 0, 0);">●</font>**  |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**   |   **<font style="color:rgb(0, 0, 0);">●</font>**  |

SIPL 控制和升级须满足后文的访问通路条件；RAW 相机的 OTA 指匹配 NITO 更新。帧信息取决于驱动输出，畸变矫正还要求有效内参及 VPI。

SHF2GY 和 SHW2GY 的 SIPL OTA 适用于匹配的 1920×1200 GW5 通路。GW6 相机使用匹配的双目驱动配置时支持 SIPL OTA。

触发模式读取支持 SDK 已登记的单目 ISX031 型号。SIPL 需要匹配的单传感器通路。使用 `camera_data_tool get-trigger-mode` 读取传感器当前触发模式。

# 四.快速上手与安装指南

## SENSING官方驱动获取

7.2 v4l2驱动：<https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_GMSL2x8_JP7.2_L4TR39.2>

7.2 sipl驱动：<https://github.com/SENSING-Technology/nvidia-jetson-camera-drivers/tree/main/Jetson%20AGX%20Thor%20Devkit/TRD1_G2A/JetPack7.2/TRD1_G2A_AGX_THOR_SIPL_JP7.2_L4TR39.2>

## SDK获取方式

1. 方式一：FAE提供deb安装包：

```plain
SENSING_sgMIX_vX.X.X.deb
```

1. 方式二：在git仓库安装

```plain
git clone https://github.com/SENSING-Technology/sgMIX.git
cd sgMIX
sudo apt install git-lfs 
git lfs install
git lfs pull
```

## 安装包安装说明

1. 将 SDK 安装包拷贝至 Jetson 设备，例如：

```plain
/home/nvidia/SENSING_sgMIX_vX.X.X.deb
```

1. 安装deb包，会自动处理依赖

```plain
sudo apt update
sudo apt install ./SENSING_sgMIX_vX.X.X.deb
```

APT 会补齐 Python 3、CMake、build-essential（GCC/G++ 和 Make）、pkg-config、GStreamer 开发包及构建所需的运行依赖；SDK 不直接依赖 `nvidia-jetpack` 包。

安装前可运行 `apt-get -s install ./SENSING_sgMIX_vX.X.X.deb` 检查计划，确认剩余依赖不会引入 BSP 更新。

安装后，SDK 文件位于 /opt/sgMIX

```plain
/opt/sgMIX/
├── include/                  # 公共 API 头文件
├── lib/                      # sgMIX 静态库、动态库及网关工具库
├── sample/                   # 示例源码，含 basic/ 入门示例
├── bin/                      # 配置、授权及维护工具
├── extensions/               # 扩展组件，按打包平台提供
│   └── sDepth_SDK/           # Thor 包中的深度应用、工具、运行库和配置
├── CMakeLists.txt            # 使用已安装的 SDK 库编译示例
├── Readme.md                 # 英文说明
└── Readme_cn.md              # 中文说明
```

运行配置脚本后会生成 `/opt/sgMIX/etc/sgmix.conf`。

1. 查询安装包版本

```plain
dpkg-deb -f SENSING_sgMIX_vX.X.X.deb Version
```

1. 安装后查询版本

```plain
dpkg-query -W -f='${Version}\n' sgmix-sdk
```

1. 卸载sgMIX

```plain
sudo apt purge sgmix-sdk
sudo rm -rf /opt/sgMIX
dpkg -l | grep sgmix-sdk
```

## sgMIX配置文件生成

安装完成后手动生成配置（安装/升级不会自动生成或覆盖配置），在目标设备安装与 BSP 匹配的 SDK 和相机驱动：V4L2 框架需要先加载驱动确保出现实际的 `/dev/videoN` 节点；SIPL 框架需要确保有配置 JSON 文件和匹配的 `libnvsipl.so`、`libnvsipl_query.so`。

配置工具不启动相机不扫描 I2C，也不通过硬件探测自动判断实际相机型号。V4L2 按 video 节点保存型号，SIPL 保存 JSON、配置名及启用链路 masks。

按平台、框架和相机型号配置后保存到 `/opt/sgMIX/etc/sgmix.conf`。

```plain
#Python 需为 3.7 或更新版本
sudo /opt/sgMIX/bin/sgmix-config.py
```

### 选择框架

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214018-79136ac8-c0da-46ec-aedd-2c32550d3a9e.png)

执行指令后会自动检测当前的硬件平台，只有NVIDIA平台才会有SIPL框架输入选项序号选择不同驱动框架。

#### V4L2：逐节点配置示例

本示例以SG3S-ISX031C-GMSL2F-Hxxx为例，连接在port7。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214099-851d30ee-f4df-4f38-8335-241e1c7a3af5.png)

启动生成配置脚本，选中V4L2框架。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448213996-eff5b36d-f46b-4f19-bb63-13004f67996c.png)

输入1并回车，脚本会自动检测当前环境下的video节点，对当前相机使用的节点进行型号配置。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214038-81e1f32b-10d2-43c7-bbf5-e0fd99d2c9d8.png)

当前相机处于video7，输入对应的选项后回车选择相机型号。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214080-8ecd3eb6-f6d6-4e4d-82cf-6fc99e9863b4.png)

回车后可以看到video7节点选中了SG3S-ISX031C-GMSL2F-Hxxx，输入s保存。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448214980-ed07992b-eb7c-40dc-a5c4-569c4b097fa7.png)

最后确认没有问题后输入y保存。

#### SIPL：逐端口配置示例

本示例以SHW3G为例，连接在port7。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215001-f07419e8-430a-4476-adc4-c9c978704f10.png)

启动生成配置脚本，选中SIPL框架。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215047-8444f3d2-0622-4c56-8ec9-8646f312742f.png)

输入2并回车，输入SIPL当前想使用的json文件，以shw3g的配置文件为例。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215043-5dfb4dbb-9518-48d2-8ba4-a9b580d3caad.png)

回车后会解析出不同port，需要针对不同的port选择对应的相机型号。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215027-f611cc66-ca10-42c5-9e82-dc73be382d5e.png)

输入7后选中port7回车后输入5选中SHW3G。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215588-4b75eef5-739d-4481-abaa-b826c53a1a80.png)

可以看到7已经变成了SHW3G，输入s回车保存。

![](https://cdn.nlark.com/yuque/0/2026/png/57985410/1793448215741-203edd1a-387e-4c24-965a-4c92704f58cb.png)

以下以包含两个 transport 的配置为例，CAM7 对应 `0:3`（transport 0、link 3）；其他配置按实际 JSON 核对：

```plain
sudo /opt/sgMIX/bin/sgmix-config.py --platform nvidia --framework sipl \
  --json /path/to/shw3g.json --config SHW3G_8CH \
  --model none --sipl-model 0:3=SHW3G
```

# 五.示例程序编译与使用说明

SDK 安装后，`/opt/sgMIX/sample` 中提供当前接口的示例源码。

使用安装目录的 CMake 模板编译：

```plain
cd /opt/sgMIX/
mkdir -p build && cd build
cmake ..
make -j"$(nproc)"
```

如果当前用户没有 `/opt/sgMIX` 的写权限，可改在用户目录构建：

```plain
cmake -S /opt/sgMIX -B "$HOME/sgmix-samples-build"
cmake --build "$HOME/sgmix-samples-build" -j6
cd "$HOME/sgmix-samples-build"
```

以下示例中的 `./程序名` 均在构建目录执行。设备或目标目录访问权限不足时使用 `sudo`。

默认编译多个完整工具和 `basic_*` 示例。

| **<font style="color:#1f2329;">源码</font>**                          | **<font style="color:#1f2329;">可执行程序</font>**                   | **<font style="color:#1f2329;">用途</font>**                     |
| ------------------------------------------------------------------- | --------------------------------------------------------------- | -------------------------------------------------------------- |
| <font style="color:#1f2329;">camera\_data\_tool.cpp</font>          | <font style="color:#1f2329;">camera\_data\_tool</font>          | <font style="color:#1f2329;">相机信息、序列号及标定数据读取</font>            |
| <font style="color:#1f2329;">camera\_frame\_capture.cpp</font>      | <font style="color:#1f2329;">camera\_frame\_capture</font>      | <font style="color:#1f2329;">V4L2、Argus、SIPL 采集、预览及畸变校正</font> |
| <font style="color:#1f2329;">camera\_ota.cpp</font>                 | <font style="color:#1f2329;">camera\_ota</font>                 | <font style="color:#1f2329;">相机固件升级及支持的 ISP/NITO 文件更新</font>   |
| <font style="color:#1f2329;">camera\_image\_controls.cpp</font>     | <font style="color:#1f2329;">camera\_image\_controls</font>     | <font style="color:#1f2329;">翻转、镜像、测试图接口验证，实时预览并尝试恢复初值</font>  |
| <font style="color:#1f2329;">camera\_calibration\_export.cpp</font> | <font style="color:#1f2329;">camera\_calibration\_export</font> | <font style="color:#1f2329;">导出 sDepth 使用的相机标定数据</font>        |
| <font style="color:#1f2329;">basic/\*.cpp</font>                    | <font style="color:#1f2329;">八个 basic\_\* 程序</font>             | <font style="color:#1f2329;">单项功能的最小 SDK 调用示例，见文末</font>       |

`SGMIX_BUILD_FULL_SAMPLES` 和 `SGMIX_BUILD_BASIC_SAMPLES` 默认均为 `ON`。只编译某一组时可分别关闭另一组；也可以只构建指定程序：

```plain
# 在已完成 cmake 配置的构建目录中执行
cmake --build . --target camera_ota -j6
```

## camera\_data\_tool：相机信息与标定数据读取

`camera_data_tool` 用于读取相机设备信息、序列号、存储的分辨率、图像配置、相机标定和 IMU 标定，支持 V4L2 和 SIPL 模式。

```plain
./camera_data_tool --help
```

### 1. 使用已保存的配置

V4L2：

```plain
./camera_data_tool --v4l2 --video /dev/video6 all
./camera_data_tool --v4l2 --video /dev/video6 get-sn
./camera_data_tool --v4l2 --video /dev/video6 get-calibration
```

SIPL：

```plain
./camera_data_tool --sipl --cam 7 all
./camera_data_tool --sipl --cam 7 get-sn
./camera_data_tool --sipl --cam 7 get-calibration
```

`--cam` 选择已启用的 Driver CAM，从 0 开始，不是模组内通道号。省略时，配置必须只选择一个传感器。同一命令只能指定一次 `--cam`。

### 2. 手动指定配置

V4L2：

```plain
./camera_data_tool --v4l2 --video /dev/video7 --model SHW3G all
```

SIPL：

```plain
./camera_data_tool --sipl --cam 7 \
  --json /path/to/camera.json \
  --config SHW3G_8CH \
  --mask "0x1000 0x0000" all
```

| **<font style="color:#1f2329;">参数</font>**                    | **<font style="color:#1f2329;">说明</font>**                                                                   |
| ------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| <font style="color:#1f2329;">--v4l2／--sipl</font>             | <font style="color:#1f2329;">选择模式，放在最前面</font>                                                               |
| <font style="color:#1f2329;">--video PATH</font>              | <font style="color:#1f2329;">V4L2 视频节点，使用 --v4l2 时必须提供</font>                                                |
| <font style="color:#1f2329;">--model MODEL</font>             | <font style="color:#1f2329;">V4L2 可选型号覆盖；省略时读取该视频节点的保存配置</font>                                              |
| <font style="color:#1f2329;">--bus N</font>                   | <font style="color:#1f2329;">V4L2 I²C 总线覆盖；本工具未提供 Sensor 地址参数，普通型号应由视频节点自动解析总线</font>                        |
| <font style="color:#1f2329;">--eeprom-addr ADDR</font>        | <font style="color:#1f2329;">本工具仅在显式指定 --model EEPROM\_CAM 时接受，用于覆盖主机可访问的 7 位 EEPROM 地址；该型号还须指定 --bus</font> |
| <font style="color:#1f2329;">--json PATH、--config NAME</font> | <font style="color:#1f2329;">SIPL 手动指定 JSON 路径及其中的配置名称</font>                                                |
| <font style="color:#1f2329;">--mask "MASK ..."</font>         | <font style="color:#1f2329;">SIPL 链路掩码，按驱动 transport 顺序用空格分隔；支持十进制或 0x 十六进制，--mask 仅指定一次</font>              |
| <font style="color:#1f2329;">--cam N</font>                   | <font style="color:#1f2329;">SIPL 已启用的 Driver CAM</font>                                                     |
| <font style="color:#1f2329;">--verbose</font>                 | <font style="color:#1f2329;">仅与 all 一起使用，增加 SDK 成员名称和完整数值精度</font>                                           |

`--json` 和 `--config` 必须同时指定；`--mask` 仅用于手动指定 SIPL 配置。端口必须对应配置中已启用的相机。

多个掩码放在同一对双引号内，顺序与对应驱动包的 `nvsipl_camera -m` 一致；`--mask` 仅指定一次。

### 3. 支持的命令

| **<font style="color:#1f2329;">命令</font>**          | **<font style="color:#1f2329;">说明</font>**                |
| --------------------------------------------------- | --------------------------------------------------------- |
| <font style="color:#1f2329;">all</font>             | <font style="color:#1f2329;">读取全部信息并汇总结果</font>           |
| <font style="color:#1f2329;">device-info</font>     | <font style="color:#1f2329;">读取设备信息及能力信息</font>           |
| <font style="color:#1f2329;">get-sn</font>          | <font style="color:#1f2329;">读取相机序列号</font>               |
| <font style="color:#1f2329;">get-resolution</font>  | <font style="color:#1f2329;">读取存储的分辨率</font>              |
| <font style="color:#1f2329;">get-image</font>       | <font style="color:#1f2329;">读取存储的图像配置</font>             |
| <font style="color:#1f2329;">get-calibration</font> | <font style="color:#1f2329;">读取相机标定；双目标定按可用数据输出</font>    |
| <font style="color:#1f2329;">get-imu</font>         | <font style="color:#1f2329;">读取 IMU 标定，不是实时 IMU 数据</font> |

`all` 会继续执行其他读取操作，即使部分项目失败。IMU 标定未烧录时显示 `NOT PROGRAMMED`，不计为失败。存在其他读取失败时，命令返回非零退出码。

实际可读项目取决于相机型号。存储的图像配置不代表当前实时采集参数；工具仅提供读取命令。

相机型号和 Sensor 信息来自配置或驱动。请分别查看 SN、分辨率和标定的读取结果。若出现 `Unavailable` 或标定缺失，请核对相机配置及所需数据是否已烧录。

以上述已保存的 S36 CAM4 配置为例，模组数据读取不需要指定采集通道：

```plain
sudo ./camera_data_tool --sipl --cam 4 device-info
sudo ./camera_data_tool --sipl --cam 4 all
```

退出码：`0` 表示成功，`1` 表示读取或初始化失败，`2` 表示用法或参数错误。

## camera\_frame\_capture：相机采集与实时预览

camera\_frame\_capture 支持 V4L2、NVIDIA Argus 和 SIPL 三种采集方式，并提供实时预览和畸变校正功能。

使用时先选择采集方式，再按需要添加预览、帧数或校正参数。

查看帮助：

```plain
./camera_frame_capture --help
```

### 1. 选择采集方式

#### V4L2（支持 YUV 相机）

通过视频设备节点采集：

```plain
./camera_frame_capture --v4l2 --video /dev/video6
```

工具默认使用配置程序保存的视频节点型号。也可以手动指定型号：

```plain
./camera_frame_capture --v4l2 --video /dev/video6 --model SG3S11AFLK
```

需要指定采集尺寸时，宽和高必须同时提供，且应匹配驱动模式：

```plain
./camera_frame_capture --v4l2 --video /dev/video6 --width 1920 --height 1080
```

#### NVIDIA Argus（支持 RAW 相机）

RAW 型号自动使用 Argus，SDK 从视频节点的数字后缀推导 Argus ID。

```plain
./camera_frame_capture --v4l2 --video /dev/video0
```

视频节点用于 SDK 初始化，Argus ID 用于选择采集源。/dev/videoN 对应 Argus ID N，两者应按实际 BSP 配置对应同一台相机。

Argus 也可以指定输出尺寸：

```plain
./camera_frame_capture --v4l2 --video /dev/video0 --width 1920 --height 1080
```

#### SIPL（支持 YUV、RAW 相机）

单通道模组使用已保存的 SIPL 配置，指定 CAM Port：

```plain
./camera_frame_capture --sipl --cam 7
```

CAM Port 从 0 开始，所选端口必须在配置中启用。

多通道模组（双目相机）必须添加 --channel N 选择一个模块内通道，或添加 --stereo 同时选择通道 0 和 1。只指定 --cam N 不会自动选择模块内通道。

配置只选择一个传感器时，可以不指定端口，由 SDK 自动解析：

```plain
./camera_frame_capture --sipl
```

多端口和模块内通道的选择方式见后面的“SIPL 通道与配置”。

### 2. 控制采集帧数和实时预览

以下参数适用于三种采集方式：

| **<font style="color:rgb(0, 0, 0);">参数</font>**     | **<font style="color:rgb(0, 0, 0);">说明</font>**                                |
| --------------------------------------------------- | ------------------------------------------------------------------------------ |
| <font style="color:rgb(0, 0, 0);">--frames N</font> | <font style="color:rgb(0, 0, 0);">每个通道采集 N 帧，必须为正整数；默认每通道 30 帧</font>          |
| <font style="color:rgb(0, 0, 0);">--view</font>     | <font style="color:rgb(0, 0, 0);">开启实时预览；未指定 --frames 时持续采集，按 Ctrl+C 停止</font> |

示例：

```plain
# V4L2 实时预览
./camera_frame_capture --v4l2 --video /dev/video6 --view

# Argus 预览 300 帧
./camera_frame_capture --v4l2 --video /dev/video0 --view --frames 300

# SIPL 实时预览
./camera_frame_capture --sipl --cam 7 --view
```

工具打印采集日志，不保存图像文件。

### 3. 启用畸变校正

在采集命令后添加 --correct：

```plain
# V4L2 预览并校正
./camera_frame_capture --v4l2 --video /dev/video6 --view --correct

# Argus 预览并校正
./camera_frame_capture --v4l2 --video /dev/video0 --view --correct

# SIPL 预览并校正
./camera_frame_capture --sipl --cam 7 --view --correct
```

启用校正后，程序需要读取当前相机的标定数据。因此应满足：

- SDK 已启用 VPI，目标设备具有匹配的 VPI 运行库；当前 JP7.2 SDK 使用 VPI 4.1.3。
- 当前相机具有有效的 Pinhole 或等距 Fisheye 标定内参。
- 采集图像尺寸与标定尺寸一致。
- 相机标定数据的读取通路正常。

#### V4L2／Argus：按需指定标定读取地址

对于无法自动解析 EEPROM 存储标定数据的相机驱动，可以使用：

| **<font style="color:rgb(0, 0, 0);">参数</font>**             | **<font style="color:rgb(0, 0, 0);">说明</font>**                               |
| ----------------------------------------------------------- | ----------------------------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">--eeprom-addr ADDR</font> | <font style="color:rgb(0, 0, 0);">覆盖主机可访问的 7 位 EEPROM 地址，范围为 0x08～0x77</font> |
| <font style="color:rgb(0, 0, 0);">--bus N</font>            | <font style="color:rgb(0, 0, 0);">覆盖 I²C 总线；普通型号存在下述 SDK 初始化限制</font>         |

例如，由视频节点自动解析 I²C 总线，使用 EEPROM 地址 0x50 读取标定并进行 Argus 校正：

```plain
./camera_frame_capture --v4l2 --video /dev/video0 \
  --eeprom-addr 0x50 --view --correct
```

地址应根据实际连接填写。地址支持十进制和十六进制，例如 80 与 0x50 等价。

地址选择顺序如下：

```plain
指定了 --eeprom-addr
    → 使用指定地址

未指定 --eeprom-addr
    → NVIDIA 平台优先使用设备树中的 eeprom-addr
    → 没有该属性时使用默认地址 0x50
```

### 4. SIPL 通道与配置

#### 选择端口和模块内通道

\--cam 选择 CAM Port，--channel 选择该模组内部的通道。是否需要显式选择通道，取决于驱动展开后的通道数量；只启用一个 CAM 不等于只有一个通道。

```plain
# 同时采集两个不同 CAM Port；所选模组均须为单采集通道
./camera_frame_capture --sipl --cam 6 --cam 7

# 采集同一 CAM Port 上的通道 0 和 1
./camera_frame_capture --sipl --cam 4 --stereo

# 只采集指定 CAM Port 上的通道 1
./camera_frame_capture --sipl --cam 4 --channel 1

# 双通道实时预览
./camera_frame_capture --sipl --cam 4 --stereo --view
```

需要区分：

- \--sipl --cam 6 --cam 7：选择两个 CAM Port。
- \--sipl --cam 4 --stereo：选择一个 CAM Port 内的通道 0 和 1。
- \--sipl --cam 4 --channel 1：选择一个 CAM Port 内的通道 1。

\--stereo 和 --channel 不能同时使用，且都要求显式指定一个 CAM Port。所选模块及驱动必须提供对应通道；端口和物理左右目的对应关系由驱动定义。

每个采集通道使用独立的工作线程和预览窗口。并行采集不代表双目帧已同步，工具不按曝光时间进行配对。

#### 多通道报错处理

如果只指定 CAM，没有选择其模块内通道，SDK 对多通道模组会报错：

```plain
Multi-channel SIPL capture requires an explicit siplChannel
```

以已启用的 CAM0 为例，按需要选择以下一种方式：

```plain
# 只预览模块内通道 0
./camera_frame_capture --sipl --cam 0 --channel 0 --view

# 只预览模块内通道 1
./camera_frame_capture --sipl --cam 0 --channel 1 --view

# 同时预览模块内通道 0 和 1，分别显示窗口
./camera_frame_capture --sipl --cam 0 --stereo --view
```

即使错误日志显示 channel=0，也不代表已经显式选择了通道；需要在命令中添加 --channel 0，或用 --stereo 选择通道 0 和 1。通道必须由该模组及驱动提供，所选 CAM 也必须已在配置和掩码中启用。

以上述 S36 CAM4、1920×1536 保存配置为例：

```plain
# 左右两路各采集 30 帧
sudo ./camera_frame_capture --sipl --cam 4 --stereo --frames 30

# 只采集通道 0；通道 1 使用 --channel 1
sudo ./camera_frame_capture --sipl --cam 4 --channel 0 --frames 30

# 左右两路分别实时预览
sudo ./camera_frame_capture --sipl --cam 4 --stereo --view
```

该示例配置输出 1920×1536 UYVY。普通采集无需有效标定；添加 --correct 时，所选通道须具备有效内参，使用 --stereo --correct 时须具备有效的左右目内参。

#### 手动指定 SIPL 配置

需要覆盖已保存的配置时，指定 JSON 文件和配置名称：

```plain
./camera_frame_capture --sipl --cam 7 \
  --json /path/to/camera.json \
  --config SHW3G_8CH \
  --mask "0x1000 0x0000"
```

| **<font style="color:rgb(0, 0, 0);">参数</font>**            | **<font style="color:rgb(0, 0, 0);">说明</font>**                                    |
| ---------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">--json PATH</font>       | <font style="color:rgb(0, 0, 0);">相机 JSON 文件路径</font>                              |
| <font style="color:rgb(0, 0, 0);">--config NAME</font>     | <font style="color:rgb(0, 0, 0);">JSON 中的配置名称</font>                               |
| <font style="color:rgb(0, 0, 0);">--mask "MASK ..."</font> | <font style="color:rgb(0, 0, 0);">按驱动 transport 顺序排列的链路掩码，整体放在同一对双引号内；仅指定一次</font> |

\--json 和 --config 必须同时提供；--mask 必须与两者一起使用。

选择 CAM Port 不会自动启用被掩码关闭的链路，端口与掩码必须匹配。

SIPL 的原生尺寸、格式和设备访问通路由驱动配置决定，不接受以下参数：

```plain
--model
--bus
--eeprom-addr
--width
--height
--isx031-ebd
```

### 5. 嵌入式数据（EBD）

| **<font style="color:rgb(0, 0, 0);">采集方式</font>** | **<font style="color:rgb(0, 0, 0);">EBD 获取方式</font>**                      |
| ------------------------------------------------- | -------------------------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">V4L2</font>     | <font style="color:rgb(0, 0, 0);">使用 --isx031-ebd 仅支持解析 ISX031 系列相机</font> |
| <font style="color:rgb(0, 0, 0);">Argus</font>    | <font style="color:rgb(0, 0, 0);">不支持上述顶部数据行提取选项</font>                    |
| <font style="color:rgb(0, 0, 0);">SIPL</font>     | <font style="color:rgb(0, 0, 0);">随帧获取驱动提供的嵌入式数据及解析字段</font>               |

#### V4L2：031 EBD 解析

```plain
./camera_frame_capture --v4l2 --video /dev/video6 --isx031-ebd
```

ISX031 系列相机不能与 --correct 或 Argus 采集同时使用，同时需要驱动支持。

#### SIPL：随帧获取 EBD

正常启动 SIPL 采集即可，EBD信息会打印到终端；多通道模组仍须显式选择通道：

```plain
./camera_frame_capture --sipl --cam 7 --frames 30
./camera_frame_capture --sipl --cam 4 --channel 0 --frames 30
```

工具会输出驱动提供的数据，包括：

| **<font style="color:rgb(0, 0, 0);">日志字段</font>**                       | **<font style="color:rgb(0, 0, 0);">说明</font>**          |
| ----------------------------------------------------------------------- | -------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">sof-tsc、eof-tsc</font>                | <font style="color:rgb(0, 0, 0);">采集开始、结束的原始 TSC</font>  |
| <font style="color:rgb(0, 0, 0);">sensor-frame</font>                   | <font style="color:rgb(0, 0, 0);">Sensor 帧序号</font>      |
| <font style="color:rgb(0, 0, 0);">sensor-timestamp-raw</font>           | <font style="color:rgb(0, 0, 0);">Sensor 原始时间戳</font>    |
| <font style="color:rgb(0, 0, 0);">top-ebd-bytes、bottom-ebd-bytes</font> | <font style="color:rgb(0, 0, 0);">顶部、底部原始 EBD 字节数</font> |
| <font style="color:rgb(0, 0, 0);">num-exposures</font>                  | <font style="color:rgb(0, 0, 0);">曝光数量</font>            |
| <font style="color:rgb(0, 0, 0);">exposure-s</font>                     | <font style="color:rgb(0, 0, 0);">曝光时间数组，单位为秒</font>     |

原始 EBD 保存在 Frame::siplMetadata 的 embeddedDataTop 和 embeddedDataBottom 中；命令行仅打印字节数，不打印原始字节内容。

实际能获取哪些数据，取决于相机、驱动和当前模式。驱动未提供原始 EBD 时，字节数为 0；解析字段无效时，相应数组为空或显示 unavailable。SIPL 不接受 --isx031-ebd 参数。

### 6. 采集日志与图像格式

V4L2 输入格式自动协商，支持 UYVY、YUYV 和 NV12；Argus 原生输入为 NV12；SIPL 原生格式由驱动决定。

当前没有命令行格式选择或帧率设置参数，帧率由采集后端和相机配置决定。SIPL 校正遇到非 NV12 图像时，会先转换为 NV12；该转换需要 GStreamer 支持。

常见日志字段如下：

| **<font style="color:rgb(0, 0, 0);">字段</font>**                    | **<font style="color:rgb(0, 0, 0);">说明</font>**                          |
| ------------------------------------------------------------------ | ------------------------------------------------------------------------ |
| <font style="color:rgb(0, 0, 0);">video</font>                     | <font style="color:rgb(0, 0, 0);">V4L2／Argus 使用的视频节点</font>              |
| <font style="color:rgb(0, 0, 0);">port、channel</font>              | <font style="color:rgb(0, 0, 0);">SIPL CAM Port 和模块内通道</font>            |
| <font style="color:rgb(0, 0, 0);">frame</font>                     | <font style="color:rgb(0, 0, 0);">SDK 帧序号</font>                         |
| <font style="color:rgb(0, 0, 0);">size</font>                      | <font style="color:rgb(0, 0, 0);">当前输出图像尺寸</font>                        |
| <font style="color:rgb(0, 0, 0);">format</font>                    | <font style="color:rgb(0, 0, 0);">当前输出图像格式</font>                        |
| <font style="color:rgb(0, 0, 0);">bytes</font>                     | <font style="color:rgb(0, 0, 0);">当前帧图像数据字节数</font>                      |
| <font style="color:rgb(0, 0, 0);">host-us</font>                   | <font style="color:rgb(0, 0, 0);">主机时间戳，单位为微秒</font>                     |
| <font style="color:rgb(0, 0, 0);">pts-ns</font>                    | <font style="color:rgb(0, 0, 0);">后端提供的 PTS，单位为纳秒</font>                 |
| <font style="color:rgb(0, 0, 0);">ebd-bytes</font>                 | <font style="color:rgb(0, 0, 0);">V4L2 提取的 EBD 字节数</font>                |
| <font style="color:rgb(0, 0, 0);">exposure-lines、exposure-s</font> | <font style="color:rgb(0, 0, 0);">V4L2 EBD 中解析出的曝光行数、曝光时间，解析有效时输出</font> |

可选元数据仅在后端提供时输出。SIPL 的 TSC 和 Sensor 原始时间戳不等同于主机时间或流水线 PTS。

退出码：0 表示成功，1 表示初始化或运行失败，2 表示命令行用法或参数解析错误。

## camera\_ota：相机固件升级与 ISP 效果文件更新

`camera_ota` 支持通过 V4L2 或 SIPL 升级固件、更新 NVIDIA ISP 调参文件，以及查询支持型号的当前运行固件版本。

执行前停止其他相机应用和所有采集任务。以下设备、CAM 编号及文件路径按实际填写，权限不足时使用 `sudo`。

查看帮助：

```plain
./camera_ota --help
```

### 1. 升级相机固件

使用已保存的配置，以下以 ISX031 为例：

```plain
# V4L2
./camera_ota --v4l2 --video /dev/video7 firmware.bin

# SIPL：实际 ISX031 单目相机
./camera_ota --sipl --cam 7 firmware.bin
```

V4L2 可手动指定型号；普通型号的 I²C 总线由视频节点解析：

```plain
./camera_ota --v4l2 --video /dev/video7 --model SG3S11AFLK firmware.bin
```

| **<font style="color:#1f2329;">类型</font>**     | **<font style="color:#1f2329;">文件与条件</font>**                  |
| ---------------------------------------------- | -------------------------------------------------------------- |
| <font style="color:#1f2329;">ISX031／S36</font> | <font style="color:#1f2329;">512 KiB .bin；写入后须给相机断电重上电</font>  |
| <font style="color:#1f2329;">GW5／GW6</font>    | <font style="color:#1f2329;">匹配的完整固件，通常为 .rom</font>           |
| <font style="color:#1f2329;">RAW相机</font>      | <font style="color:#1f2329;">nvidia平台的isp效果文件，通常为 .nito</font> |

文件须匹配实际相机、BSP 和框架。可使用本地路径或FAE提供的下载地址，不直接接受普通 URL。

V4L2 S36 从任一对应视频节点发起时，依次升级左右目，两个节点的保存型号均须为 S36。SIPL S36 要求匹配的 MAX9295D 双 ISX031 通路，按物理左目、右目顺序升级，写入后执行右目地址配置。当前 S36 升级不执行完整固件读回比对；须给相机断电重上电，再重新初始化并确认左右目版本。

### 2. 更新 NVIDIA ISP 参数

```plain
# V4L2：SHW3G、SHW5G、S56 支持 .isp／.nito
./camera_ota --v4l2 --video /dev/video7 --model SHW3G /path/to/update.nito

# SIPL：匹配的单目 RAW 模组，更新已有 NITO
./camera_ota --sipl --cam 7 /path/to/SHW3G.nito
```

文件必须非空且不超过 16 MiB。

- V4L2：成功表示文件安装完成，需重新加载使用该文件的 Argus 进程或服务；NITO 使用方须配置匹配的 `NVCAMERA_NITO_PATH`。
- SIPL：支持匹配的单目 RAW 及 S56／S56C 双目 RAW 模组更新已有 NITO，包含重新加载、出帧验证和失败回滚；采集保持停止。

### 3. 手动指定 SIPL 配置

```plain
./camera_ota --sipl --cam 7 \
  --json /path/to/camera.json --config SHW3G_8CH \
  --mask "0x1000 0x0000" /path/to/SHW3G.nito
```

`--json` 和 `--config` 同时提供，`--mask` 按驱动 transport 顺序填写且只指定一次。OTA 一次选择一个已启用的 CAM；省略 `--cam` 时，配置必须只选择一个传感器。

### 4. 查询当前固件版本

将最后的文件参数替换为 `--get-version`：

```plain
./camera_ota --v4l2 --video /dev/video7 --get-version
./camera_ota --sipl --cam 7 --get-version
```

该命令不查询 NITO 版本。

```plain
Firmware version (running): <当前运行版本>
```

ISX031／S36 固件写入后须断电重上电，再确认新运行版本。V4L2 S36 的 `--get-version` 只读取指定节点对应的一目，应分别查询两个视频节点；SIPL S36 一次返回 channel0、channel1 的当前运行版本。

退出码：`0` 成功，`1` 操作失败，`2` 用法或参数错误。固件写入后的验证失败，不代表固件未写入。

## camera\_image\_controls：图像接口验证

`camera_image_controls` 支持 V4L2、NVIDIA Argus 和 SIPL，用于实时预览并检查相机图像控制接口。命令末尾必须指定 `image`。

以初始状态为正常实景、翻转和镜像均关闭为例，画面依次为：**正常实景 → 上下翻转 → 恢复实景 → 左右镜像 → 恢复实景 → 测试图**。程序先预览约 3 秒，再依次测试 Flip、Mirror、Testpattern；每组先关闭、再开启，各显示约 3 秒，随后恢复该项初值。全部测试结束后，恢复初始设置、关闭预览并停止采集。

查看帮助：

```plain
./camera_image_controls --help
```

### 1. 选择采集方式

```plain
# V4L2
./camera_image_controls --v4l2 --video /dev/video6 image

# NVIDIA Argus
./camera_image_controls --v4l2 --video /dev/video0 image

# SIPL，默认模块内通道 0
./camera_image_controls --sipl --cam 7 image

# SIPL，指定模块内通道
./camera_image_controls --sipl --cam 4 --channel 1 image
```

相机型号及 SIPL 配置使用已保存的配置。RAW 型号自动使用 Argus，`/dev/videoN` 自动对应 sensor-id `N`。SIPL 指定 `--channel` 时必须指定 `--cam`。

### 2. 验证功能

| **<font style="color:rgb(0, 0, 0);">模式</font>** | **<font style="color:rgb(0, 0, 0);">检查内容</font>**                   |
| ----------------------------------------------- | ------------------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">image</font>  | <font style="color:rgb(0, 0, 0);">翻转、镜像、测试图，共 3 组 Get／Set 接口</font> |

测试图的具体图案取决于相机型号。

### 3. 测试流程与结果

程序读取并保存初值，依次设置测试值，每次成功设置后显示 3 秒，再读取状态比较，每组测试后尝试恢复初始状态。Ctrl+C 或运行异常时也会尝试恢复并停止采集。

运行前停止其他相机应用，并在支持 GStreamer 预览的板端桌面环境中运行。

实际支持的项目取决于相机型号。不支持、读回不一致或恢复失败都会导致最终结果为 `FAIL`。初值读取失败时，会跳过该项设置。

### 4. 预期输出

终端会打印设置结果、读回比较和恢复结果。全部通过时，典型输出如下：

```plain
[PASS] Flip Get/Set
[PASS] Mirror Get/Set
[PASS] Testpattern Get/Set
[RESULT] PASS pairs_passed=3/3 failures=0 (visual effects require observation)
```

`PASS` 表示接口调用、读回比较和恢复检查通过，画面效果仍需人工确认。

退出码：`0` 全部通过，`1` 运行或检查失败，`2` 参数错误，`130` 中断。

## basic\_\*：最小 SDK 调用示例

`sample/basic/` 下每个 `.cpp` 独立演示一项功能，适合复制到自己的项目中。它们只使用公开 SDK 接口和 C++17 标准库，使用匹配的已保存配置；不提供完整工具的可选命令行参数。

### 1. 编译

开头的默认构建已经包含全部八个 `basic_*` 程序。只编译这些最小示例时执行：

```plain
cmake -S /opt/sgMIX -B "$HOME/sgmix-basic-build" \
  -DSGMIX_BUILD_FULL_SAMPLES=OFF
cmake --build "$HOME/sgmix-basic-build" -j6
cd "$HOME/sgmix-basic-build"
```

也可单独编译一个源码文件，例如：

```plain
g++ -std=c++17 /opt/sgMIX/sample/basic/sipl_capture.cpp \
  -o basic_sipl_capture \
  -I/opt/sgMIX/include -L/opt/sgMIX/lib \
  -Wl,-rpath,/opt/sgMIX/lib -lsgmix -pthread
```

### 2. 源码及使用方式

| **<font style="color:rgb(0, 0, 0);">源码</font>**                            | **<font style="color:rgb(0, 0, 0);">运行命令</font>**                                    | **<font style="color:rgb(0, 0, 0);">行为及主要接口</font>**                                                     |
| -------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------- |
| <font style="color:rgb(0, 0, 0);">basic/v4l2\_capture.cpp</font>           | <font style="color:rgb(0, 0, 0);">./basic\_v4l2\_capture /dev/video6</font>          | <font style="color:rgb(0, 0, 0);">读取 30 帧并打印帧号、尺寸和字节数；输入格式自动协商 UYVY／YUYV／NV12，默认不强制输出格式，尺寸／帧率交由协商</font> |
| <font style="color:rgb(0, 0, 0);">basic/sipl\_capture.cpp</font>           | <font style="color:rgb(0, 0, 0);">./basic\_sipl\_capture 7</font>                    | <font style="color:rgb(0, 0, 0);">从 CAM7 单传感器模组读取 30 帧，尺寸和格式由 SIPL 驱动决定</font>                           |
| <font style="color:rgb(0, 0, 0);">basic/sipl\_stereo\_capture.cpp</font>   | <font style="color:rgb(0, 0, 0);">./basic\_sipl\_stereo\_capture 4</font>            | <font style="color:rgb(0, 0, 0);">CAM4 的通道 0／1 各使用一个对象和线程，分别读取 30 帧</font>                               |
| <font style="color:rgb(0, 0, 0);">basic/v4l2\_read\_sn.cpp</font>          | <font style="color:rgb(0, 0, 0);">./basic\_v4l2\_read\_sn /dev/video6</font>         | <font style="color:rgb(0, 0, 0);">初始化后调用 SG\_GetCameraSerialNumber() 读取序列号，不启动采集</font>                  |
| <font style="color:rgb(0, 0, 0);">basic/sipl\_read\_calibration.cpp</font> | <font style="color:rgb(0, 0, 0);">./basic\_sipl\_read\_calibration 4</font>          | <font style="color:rgb(0, 0, 0);">调用 SG\_GetCalibration() 读取标定，打印内参；有右目及双目外参时一并打印，示例不显式启动采集</font>       |
| <font style="color:rgb(0, 0, 0);">basic/v4l2\_correct\_capture.cpp</font>  | <font style="color:rgb(0, 0, 0);">./basic\_v4l2\_correct\_capture /dev/video6</font> | <font style="color:rgb(0, 0, 0);">请求 NV12 并设置 correctDistortion = true，读取 30 帧校正后的图像</font>              |
| <font style="color:rgb(0, 0, 0);">basic/v4l2\_image\_flip.cpp</font>       | <font style="color:rgb(0, 0, 0);">./basic\_v4l2\_image\_flip /dev/video6 1 0</font>  | <font style="color:rgb(0, 0, 0);">设置上下翻转、左右镜像，采集 30 帧后恢复原值</font>                                        |
| <font style="color:rgb(0, 0, 0);">basic/sipl\_image\_flip.cpp</font>       | <font style="color:rgb(0, 0, 0);">./basic\_sipl\_image\_flip 7 1 0</font>            | <font style="color:rgb(0, 0, 0);">对 SIPL 已启用的单传感器 CAM 执行翻转、镜像及恢复</font>                                  |

采集类示例直接演示以下调用顺序：

```plain
SG_Init → SG_StartCapture → SG_GetFrame → SG_StopCapture
```

可在采集循环的注释位置处理 `frame.data`，并按 `pixelFormat`、尺寸和 `planes` 解释数据。同一个 `Frame` 的下一次读取会替换其内容；跨线程或延后使用时，应复制或移动 `Frame` 以保留数据所有权。

最小示例不打开预览窗口或保存图像。校正示例要求匹配的 VPI 运行库、目标通道的有效内参以及与标定一致的采集尺寸。双通道顺序来自 Query，独立读取不代表曝光同步配对。

这些示例不提供 `--help` 参数；不传参数时会打印其用法。退出码：`0` 表示成功，`1` 表示运行失败，`2` 表示用法或参数错误。

### 3. 翻转、镜像控制

使用已保存的配置，参数形式与其他最小示例一致：

```plain
# 最后两个参数依次为上下翻转、左右镜像：0 关闭，1 开启
./basic_v4l2_image_flip /dev/video6 1 0
./basic_sipl_image_flip 7 1 0
```

调用流程：初始化 → 启动采集并取得首帧 → 读取全部原值 → 设置并回读 → 采集 30 帧 → 逆序恢复原值 → 停止采集。示例在设置参数后打印 30 帧。

程序打印 `before`、`requested`、`readback` 以及恢复结果。设置失败也可能已经改变设备，因此会尝试恢复该参数；后续回读、采集失败或收到 SIGINT／SIGTERM 时，也执行恢复和停流。恢复或停流失败会单独报错并返回非零退出码。

回读按公开接口返回的布尔状态校验，要求与请求值一致；画面效果可通过 `camera_image_controls` 实时预览确认。

# SDK 功能与接口说明

SDK 使用 C++17，公开类型和 CameraControl 位于 sgmix 命名空间；接口以include/SG\_CameraControl.h、include/SG\_CameraTypes.h 为准。除显式内参矫正外，调用前须成功初始化；失败返回 false 并通过 error 输出错误信息。

## 初始化

函数原型：

```plain
bool SG_Init(const CameraInitConfig& config, std::string& error);
```

V4L2 传入 `CameraConfig`。通常指定视频节点即可，型号可从保存配置读取。手动指定 I²C 总线时，通常还要提供主机可见的 7 位 Sensor 地址。`EEPROM_CAM` 必须显式指定总线。地址覆盖规则见 `SG_CameraTypes.h`。

```plain
sgmix::CameraConfig config;
config.videoDevice = "/dev/video0";
// 可显式覆盖已保存型号：
// config.model = "SG3S-ISX031C-GMSL2-Hxxx";
sgmix::CameraControl camera;
std::string error;
if (!camera.SG_Init(config, error)) {
    // 处理error，停止后续操作。
}
```

SIPL 传入 SiplCameraConfig。下面从已保存的 SIPL 配置绑定 CAM7。

```cpp
sgmix::SiplCameraConfig config;
config.cameraPort = 7;
if (!camera.SG_Init(config, error)) {
    // 处理初始化错误。
}
```

`cameraPort` 是应用掩码前的 Driver CAM 编号，0 表示 CAM0。仅启用一个传感器时可省略。使用相同配置的对象共享会话，切换配置前须停止采集并释放所有绑定对象。JSON 和链路掩码用法见前文的 SIPL 配置示例。

## 视频流控制

函数原型：

```plain
/** @brief 在已初始化的相机上启动采集。 */
bool SG_StartCapture(const CaptureConfig& config, std::string& error);

/** @brief 消费新帧；timeoutMs为毫秒，0立即轮询，失败清空frame。 */
bool SG_GetFrame(Frame& frame, std::uint32_t timeoutMs, std::string& error);

/** @brief 停止采集并释放资源，可重复调用；析构也会停止。 */
bool SG_StopCapture(std::string& error);
```

先调用 `SG_Init` 初始化相机，再调用 `SG_StartCapture` 开始采集。循环调用 `SG_GetFrame` 取帧，用完后调用 `SG_StopCapture`。

- **采集参数**。SDK 根据初始化配置和型号选择 V4L2、Argus 或 SIPL。尺寸和帧率为 0 时使用来源配置。V4L2 和 Argus 的宽高应同时指定或同时留为 0。SIPL 显式指定的尺寸和帧率须匹配 JSON，多通道模组每个通道使用一个 `CameraControl` 对象并显式设置 `siplChannel`。
- **帧数据**。`Frame` 持有独立的 CPU 数据，下一次取帧或停流后仍可使用。实时采集只保留一张待取帧，处理慢时会丢弃旧帧。异步启动错误可能由 `SG_GetFrame` 报告。
- **并发**。同一对象只允许一个线程取帧，另一线程可用 `SG_StopCapture` 中断等待。其他操作须串行调用。重新初始化会先停流，失败后需要重新初始化才能采集。

像素格式、平面布局和时间戳字段见 `SG_CameraTypes.h`。采集命令见前文的 `camera_frame_capture` 示例。

## 畸变矫正

函数原型：

```plain
/**
 * @brief 同步矫正已有NV12帧，不自动转换像素格式。
 * @param input 输入帧，像素和元数据保持不变。
 * @param output 独立的紧密排列NV12输出，不能与input为同一对象。
 * @param error 输出错误信息字符串。
 * @param intrinsics 可选单sensor内参，显式提供时无需初始化或访问EEPROM。
 * @return 成功返回true，失败返回false。
 */
 bool SG_CorrectFrame(const Frame& input, Frame& output, std::string& error, const std::optional<CameraCalibration::Intrinsics>& intrinsics = std::nullopt);
```

输入须为双平面 NV12，尺寸与标定一致。设备需要启用 VPI，运行库版本须为 4.1.3 或以上且低于 5.0。支持针孔和等距鱼眼标定。

省略内参时，SDK 从已初始化的相机读取并缓存内参。双目应使用对应通道的内参，无法自动确定通道时需显式传入。采集时可设置 `CaptureConfig::correctDistortion` 返回校正后的 NV12 帧。

## 获取相机信息

函数原型：

```plain
/**
 * @brief 获取设备身份、有效通路、镜头、SN及后端能力。
 * @param info  输出相机信息结构体。
 * @param error 输出错误信息字符串。
 * @return 成功返回 true，失败返回 false。
 */
 bool SG_GetDeviceInfo(DeviceInfo& info, std::string& error);
```

DeviceInfo 的可选字段须先检查是否有值；能力标志不等于已授权或运行时升级检查通过。使用可选 sensor.endpoint 查询实际通路：

```plain
sgmix::DeviceInfo info;
if (camera.SG_GetDeviceInfo(info, error) && info.sensor.endpoint) {
    const auto address = info.sensor.endpoint->address; // 主机可见7位地址
    const auto bus = info.sensor.endpoint->bus;
}
```

## 获取图像信息

函数原型：

```plain
/** @brief 读取存储的图像配置。 */
bool SG_GetImageInfo(ImageInfo& info, std::string& error);
```

ImageInfo 包含width、height、framesPerSecond、pixelFormat、triggerMode和embeddedDataEnabled，表示存储配置，不是当前采集协商结果。

## 获取相机SN码

函数原型：

```plain
/**
 * @brief 获取当前相机的序列号。
 * @param serialnumber 输出相机序列号字符串。
 * @param error        输出错误信息字符串。
 * @return 成功返回 true，失败返回 false。
 */
 bool SG_GetCameraSerialNumber(std::string& serialnumber, std::string& error);
```

SN 最多读取 32 字节。

## 相机分辨率读取

函数原型：

```plain
/** @brief 读取存储的相机分辨率。 */
bool SG_GetCameraResolution(uint16_t& width, uint16_t& height, std::string& error);
```

## 相机内、外参

函数原型：

```plain
/**
 * @brief 按需读取相机标定。
 * @param calibration 输出标定结构，left为必需内参。
 * @param error 输出错误信息字符串。
 * @return 成功返回true，必需数据读取失败返回false。
 */
 bool SG_GetCalibration(CameraCalibration& calibration, std::string& error);
```

左目内参是必需数据。右目内参、双目外参和 RMS 为可选值。SIPL ISX031 须使用实际型号和匹配的配置。读取 Flash 前须停止共享会话的全部采集。若恢复失败，释放所有共享对象并重新初始化。

通过 CameraCalibration::right 和 stereo 获取可选右内参和双目外参：

```plain
sgmix::CameraCalibration calibration;
if (camera.SG_GetCalibration(calibration, error) && calibration.right && calibration.stereo) {
    const auto rightFx = calibration.right->fx;
    const auto translation = calibration.stereo->translation; // 毫米
}
```

SIPL双目模组通过siplChannel分别采集，不自动组成同步帧对。

## IMU标定读取

函数原型：

```plain
/** @brief 读取IMU标定。 */
bool SG_GetImuCalibration(ImuCalibration& calibration, std::string& error);
```

ImuCalibration calibration{}; 包含噪声、偏置、标定矩阵及相机到IMU变换；平移单位为米，时间偏移为秒。未烧录或数据无效返回错误。

## 获取图像翻转

SIPL图像控制须先启动当前通道采集，要求已核实的MAX96717/F通路，或已适配且具有独立Sensor映射的S56/S56C、S36 MAX9295D通路。S56C产品型号使用S56，siplChannel选择通道；S56/S56C MAX9295D图像Set要求只启用一个双目模组、仅一个活动采集通道。

函数原型：

```plain
/**
 * @brief 获取当前图像翻转（上下反转）状态。
 * @param state 输出布尔值，true 表示开启翻转。
 * @param error 输出错误信息字符串。
 */
 bool SG_GetCameraImageFlip(bool& state, std::string& error);
```

## 获取图像镜像

函数原型：

```plain
/**
 * @brief 获取当前图像镜像（左右镜像）状态。
 * @param state 输出布尔值，true 表示开启镜像。
 * @param error 输出错误信息字符串。
 */
 bool SG_GetCameraImageMirror(bool& state, std::string& error);
```

## 获取测试图像状态

函数原型：

```plain
/**
 * @brief 获取测试图像（test pattern）输出状态。
 * @param state 输出布尔值，true 表示开启测试图像。
 * @param error 输出错误信息字符串。
 */
 bool SG_GetCameraImageTestpattern(bool& state, std::string& error);
```

## 设置图像翻转

函数原型：

```plain
/**
 * @brief 设置图像翻转状态。
 * @param state 布尔值，true 表示开启翻转。
 * @param error 输出错误信息字符串。
 */
 bool SG_SetCameraImageFlip(bool state, std::string& error);
```

## 设置图像镜像

函数原型：

```plain
/**
 * @brief 设置图像镜像状态。
 * @param state 布尔值，true 表示开启镜像。
 * @param error 输出错误信息字符串。
 */
 bool SG_SetCameraImageMirror(bool state, std::string& error);
```

## 输出测试图像

函数原型：

```plain
/**
 * @brief 设置是否开启测试图像输出。
 * @param state 布尔值，true 表示开启测试图像。
 * @param error 输出错误信息字符串。
 */
 bool SG_SetCameraImageTestpattern(bool state, std::string& error);
```

## OTA

函数原型：

```plain
/** @brief 升级相机固件或更新受支持的效果文件。 */
bool SG_PerformOTAUpdate(std::string urlPath, std::string& error);
/** @brief GW6 全量升级时传入 true。 */
bool SG_PerformOTAUpdate(std::string urlPath, std::string& error, bool fullRom);
```

传入与相机匹配的升级文件。GW6 默认升级分量，传入 `fullRom=true` 执行受支持的全量升级。

升级前停止采集，SIPL 须停止共享会话的全部采集。升级后按型号要求重新初始化或给相机断电重上电，并确认运行版本。文件要求、完整操作步骤和示例见前文的 `camera_ota` 章节。

## 恢复 S36 右目地址

函数原型：

```plain
/**
 * @brief 配置 S36 右目原生地址为 0x1B，不擦除或写入固件。
 * @param error 输出错误信息字符串。
 * @return 配置序列执行成功返回 true，失败返回 false。
 */
bool SG_RestoreS36RightAddress(std::string& error);
```

仅支持匹配的 SIPL 双 ISX031／MAX9295D S36 通路，调用前停止共享会话全部采集。尝试恢复后释放全部共享对象，给相机断电重上电，再重新初始化并确认地址及版本。

## 离线授权

SDK 在调用 ISP 接口时自动加载绑定本机的离线许可证。

## 获取相机固件版本号

函数原型：

```plain
/**
 * @brief 获取当前相机固件版本。
 * @param version 输出固件版本字符串。
 * @param error   输出错误信息字符串。
 * @return 成功返回 true，失败返回 false。
 */
 bool SG_GetFirmwareVersion(std::string& version, std::string& error);
```

初始化后可查询当前运行的固件版本。SIPL 下查询 ISX031 或 S36 前，须停止共享会话的全部采集。SIPL S36 返回左右目版本，SDT2BCO1 要求同一 GMSL I²C 总线只启用一个模组。

## 相机亮度控制

**ISX031 和 S36 均支持 V4L2 和 SIPL ISP 参数控制。S36 在 SIPL 下要求匹配的 MAX9295D 双 ISX031 通路。**

| 通路与机型                                                | 支持的 ISP 参数                                                |
| ---------------------------------------------------- | --------------------------------------------------------- |
| V4L2：SDK 已登记的 ISX031 型号、S36                          | 对比度、饱和度、锐度、曝光模式、手动曝光时间、模拟增益、白平衡模式和 RGB 白平衡。               |
| V4L2：SHW3G、SHW5G、S56                                 | 亮度、对比度、饱和度、锐度、降噪、曝光模式、手动曝光时间和模拟增益。                        |
| SIPL：SDK 已登记的 ISX031 型号、匹配 MAX9295D 双 ISX031 通路的 S36 | 对比度、饱和度、锐度、曝光模式、手动曝光时间、模拟增益、白平衡模式和 RGB 白平衡。调用前须启动对应通道的采集。 |

SIPL S36 的控制作用于正在采集的指定通道。NVIDIA 的百分比类 ISP 参数通过调优文件读写，SDK 不会自动重载 Argus。实际参数范围取决于相机模式和固件。启用网络授权校验的 SDK 需要对应功能的授权。

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取相机亮度设置（范围 0-100%）。 */
bool SG_GetCameraBrightness(int& brightness_percent, std::string& error);

/** @brief 设置相机亮度（范围 0-100%）。 */
bool SG_SetCameraBrightness(int brightness_percent, std::string& error);
```

## 相机对比度控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取相机对比度设置（范围 0-100%）。 */
bool SG_GetCameraContrast(int& contrast_percent, std::string& error);

/** @brief 设置相机对比度（范围 0-100%）。 */
bool SG_SetCameraContrast(int contrast_percent, std::string& error);
```

## 相机饱和度控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取相机饱和度设置（范围 0-100%）。 */
bool SG_GetCameraSaturation(int& saturation_percent, std::string& error);

/** @brief 设置相机饱和度（范围 0-100%）。 */
bool SG_SetCameraSaturation(int saturation_percent, std::string& error);
```

## 相机锐化控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取相机锐度设置（范围 0-100%）。 */
bool SG_GetCameraSharpness(int& sharpness_percent, std::string& error);

/** @brief 设置相机锐度（范围 0-100%）。 */
bool SG_SetCameraSharpness(int sharpness_percent, std::string& error);
```

## 相机降噪控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取相机去噪等级（范围 0-100%）。 */
bool SG_GetCameraDenoise(int& denoise_percent, std::string& error);

/** @brief 设置相机去噪等级（范围 0-100%）。 */
bool SG_SetCameraDenoise(int denoise_percent, std::string& error);
```

## 相机曝光控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 设置曝光模式（自动/手动）。 */bool SG_SetExposureMode(bool state, std::string& error);

/** @brief 获取当前曝光模式（自动/手动）。 */
bool SG_GetExposureMode(bool& state, std::string& error);

/**
 * @brief 读取当前曝光时间控制值，不是逐帧EBD数据。
 * @param val 输出曝光时间，毫秒。
 * @param error 输出错误信息字符串。
 * @return 成功返回true，失败返回false。
 * @note 需对应功能授权。
 */
 bool SG_GetExposureTime(float& val, std::string& error);

/** @brief 设置手动曝光时间（毫秒）。 */
bool SG_SetManualExposure(float val, std::string& error);
```

state为true表示自动，false表示手动；设置曝光时间前先切手动模式。

## 相机白平衡控制

函数原型：

```plain
/** @note 该功能需要网络授权才能使用。*//** @brief 获取白平衡模式（自动/手动）。 */
bool SG_GetCameraWhiteBalanceMode(bool& wb_mode, std::string& error);

/** @brief 设置白平衡模式（自动/手动）。 */
bool SG_SetCameraWhiteBalanceMode(bool wb_mode, std::string& error);

/** @brief 获取手动白平衡的 RGB 增益值。 */
bool SG_GetCameraWhiteBalance(int& wb_R, int& wb_G, int& wb_B, std::string& error);

/** @brief 设置手动白平衡的 RGB 增益值。 */
bool SG_SetCameraWhiteBalance(int wb_R, int wb_G, int wb_B, std::string& error);
```

wb\_mode为true表示自动，false表示手动；设置RGB增益前先切手动模式。

## 相机模拟增益控制

函数原型：

```plain
/** @brief 获取当前传感器的模拟增益值。 */
bool SG_GetGainValue(int& val, std::string& error);

/** @brief 设置手动模拟增益值。 */
bool SG_SetManualGain(int val, std::string& error);
```

设置增益前先切手动曝光模式。

# 已知问题

### V4L2：031内参读取与首次开流的地址映射

使用依赖首次开流完成地址映射的V4L2驱动时，应先完成一次正常开流，再读取031标定。SIPL的访问准备由SDK内部完成，Flash读取前须停止共享会话全部采集。
