/**
 *  @file        BeeSystemParam.java
 *  @brief       BeeSDK初始化参数.
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

/// BeeSDK初始化参数类.
public class BeeSystemParam {
    /// 平台类型.
    public int     platform_type;
    /// 集成SDK的App名.
    public String  app_name;
    /// 集成SDK的App版本.
    public String  app_version;
    /// 操作系统版本等信息.
    public String  system_info;
    /// 机器码，作为设备唯一标识.
    public String  machine_code;
    /// 日志路径，当logVolumeCount=0时，日志名以日期为后缀，当前logVolumeCount>0时，日志名以卷号为后缀.
    public String  log_path;
    /// 日志级别.
    public int     log_level;
    /// 单个日志最大行数(logVolumeCount=0).
    public int     log_max_line;
    /// 日志卷数.
    public int     log_volume_count;
    /// 每卷日志的大小(logVolumeCount>0)，单位为KB.
    public int     log_volume_size;
    /// 最大会话数.
    public int     session_count;
    /// 是否使能实时监控.
    public boolean enable_statusd;
    /// 是否开启硬件编码.
    public boolean enable_video_encoder_hw;
    /// 是否开启硬件解码.
    public boolean enable_video_decoder_hw;
}
