/**
 *  @file        BeeSystemDefine.java
 *  @brief       BeeSDK全局声明文件
 *  @author      fangdali
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */
package com.sohu.tv.bee;

/// 系统全局定义
public class BeeSystemDefine {
    /// 错误码.
    public enum BeeErrorCode {
        kBeeErrorCode_Success,                              //!< 成功.
        kBeeErrorCode_General_Error,                        //!< 普通错误.
        kBeeErrorCode_Invalid_Param,                        //!< 非法参数.
        kBeeErrorCode_Invalid_State,                        //!< 非法状态.
        kBeeErrorCode_Null_Pointer,                         //!< 空指针.
        kBeeErrorCode_Insufficient_Session,                 //!< 会话不足.
        kBeeErrorCode_Invalid_Session,                      //!< 非法会话.
        kBeeErrorCode_Engine_Internal_Error,                //!< 脚本引擎内部错误.
        kBeeErrorCode_Engine_Script_Error,                  //!< 脚本语法错误.
        kBeeErrorCode_Engine_Not_Loaded,                    //!< 脚本未加载.
        kBeeErrorCode_Engine_Reject,                        //!< 脚本拒绝.
        kBeeErrorCode_Give_Up,                              //!< 放弃.
        kBeeErrorCode_Not_Implemented,                      //!< 未实现.
        kBeeErrorCode_Timeout,                              //!< 超时.
        kBeeErrorCode_Service_Not_Started,                  //!< 服务未启动.
        kBeeErrorCode_Session_Not_Opened,                   //!< 会话未打开.
        kBeeErrorCode_Not_Connected,                        //!< 未连接.
        kBeeErrorCode_Error_Http_Status,                    //!< 错误的Http状态码.
        kBeeErrorCode_Error_Data,                           //!< 错误数据.
        kBeeErrorCode_Error_State_Machine,                  //!< 状态机错误.
        kBeeErrorCode_State_Machine_Interrupted,            //!< 状态机中断.
        kBeeErrorCode_State_Machine_Finished,               //!< 状态机结束.
        kBeeErrorCode_Resolve_Fail,                         //!< DNS解析失败.
        kBeeErrorCode_Connect_Fail,                         //!< 连接失败.
        kBeeErrorCode_Tls_Shakehand_Fail,                   //!< Tls握手失败.
        kBeeErrorCode_Write_Fail,                           //!< 写数据失败.
        kBeeErrorCode_Read_Fail,                            //!< 读数据失败.
        kBeeErrorCode_Read_Http_Header_Fail,                //!< 读Http头失败.
        kBeeErrorCode_Read_Http_Content_Fail,               //!< 读Http体失败.
        kBeeErrorCode_Read_Http_Chunk_Size_Fail,            //!< 读Http Chunk大小失败.
        kBeeErrorCode_Read_Http_Chunk_Body_Fail,            //!< 读Http Chunk体失败.
        kBeeErrorCode_Crypto_Error,                         //!< 加解密错误.
        kBeeErrorCode_Decompress_Error,                     //!< 解压缩错误.
        kBeeErrorCode_Signature_Error,                      //!< 签名错误.
        kBeeErrorCode_Invalid_Protocol_Mark,                //!< 非法协议.
        kBeeErrorCode_Invalid_Message,                      //!< 非法消息.
        kBeeErrorCode_Eof,                                  //!< 结束标识.
        kBeeErrorCode_Not_Enough_Memory,                    //!< 内存不足.
        kBeeErrorCode_Lws_Error,                            //!< libwebsockets错误.
        kBeeErrorCode_Webrtc_Create_Peer_Connection_Fail,   //!< WebRTC创建PeerConnection失败.
        kBeeErrorCode_Webrtc_Open_Video_Capture_Fail,       //!< WebRTC打开摄像头失败.
        kBeeErrorCode_Webrtc_Create_Local_Stream_Fail,      //!< WebRTC创建本地流失败.
        kBeeErrorCode_Webrtc_Add_Stream_Fail,               //!< WebRTC添加流失败.
        kBeeErrorCode_Webrtc_Error_Media,                   //!< WebRTC媒体错误.
        kBeeErrorCode_OpenGL_Error,                         //!< OpenGL错误.
        kBeeErrorCode_Push_Fail,                            //!< 推流失败.
        kBeeErrorCode_Pull_Fail,                            //!< 拉流失败.
        kBeeErrorCode_Capture_Not_Set,                      //!< 摄像头未设置.
        kBeeErrorCode_Renderer_Not_Set,                     //!< 渲染器未设置.
        kBeeErrorCode_Member_Existed,                       //!< 成员已存在.
        kBeeErrorCode_Member_Not_Exist,                     //!< 成员不存在.
        kBeeErrorCode_Webrtc_Ice_Fail,                      //!< WebRTC ICE失败。
        kBeeErrorCode_Existed,                              //!< 已经存在.
        kBeeErrorCode_Not_Found,                            //!< 不存在.
        kBeeErrorCode_Service_Not_Supported,                //!< 不支持的业务.
        kBeeErrorCode_Service_Not_Registered,               //!< 业务未注册.
        kBeeErrorCode_Create_Device_Info_Failed,            //!< 创建设备信息失败.
        kBeeErrorCode_No_Camera,                            //!< 没有摄像头.
        kBeeErrorCode_Open_Camera_Failed,                   //!< 打开摄像头失败.
        kBeeErrorCode_No_Network,                           //!< 无网络
        kBeeErrorCode_Network_Not_Allow,                    //!< 网络不允许
        kBeeErrorCode_Count,                                //!< 错误码总数.
        kBeeErrorCode_Not_Compatible                        //!< 不兼容.
    }

    /// 平台类型.
    public enum BeePlatformType {
        kPlatformType_None,                 //!< 空类型.
        kPlatformType_PC,                   //!< Windows电脑.
        kPlatformType_Mac,                  //!< 苹果电脑.
        kPlatformType_IPhone,               //!< 苹果手机.
        kPlatformType_IPad,                 //!< 苹果平板.
        kPlatformType_Android_Phone,        //!< Android手机.
        kPlatformType_Android_Pad,          //!< Android平板.
        kPlatformType_Android_TV,           //!< Android电视.
        kPlatformType_Android_Router,       //!< Android路由器.
        kPlatformType_Android_Box           //!< Android盒子.
    }

    /// 网络类型.
    public enum BeeNetType {
        kNetType_None,                      //!< 空类型.
        kNetType_WireLine,                  //!< 有线网络.
        kNetType_Wifi,                      //!< 无线局域网.
        kNetType_2G,                        //!< 2G网络.
        kNetType_3G,                        //!< 3G网络.
        kNetType_4G                         //!< 4G网络.
    }

    /// 日志级别.
    public enum BeeLogLevel {
        kLogLevel_Fatal,                    //!< 致命错误.
        kLogLevel_Error,                    //!< 错误.
        kLogLevel_Warn,                     //!< 警告.
        kLogLevel_Trace,                    //!< 跟踪.
        kLogLevel_Info,                     //!< 信息.
        kLogLevel_Debug,                    //!< 调试.
        kLogLevel_All,                      //!< 所有级别.
        kLogLevel_Count                     //!< 日志级别总数.
    }
}
