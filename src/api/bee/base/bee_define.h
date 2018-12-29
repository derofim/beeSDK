/**
 *  @file        bee_define.h
 *  @brief       BeeSDK全局声明文件.
 *  @author      HeZhen
 *  @copyright   sohu-inc
 *  @version     2.0.0.1
 */

#ifndef __BEE_DEFINE_H__
#define __BEE_DEFINE_H__

#ifdef  WIN32
#ifdef  BEE_USE_AS_DLL
#ifdef  BEE_EXPORTS
#define BEE_API __declspec(dllexport)
#else
#define BEE_API __declspec(dllimport)
#endif
#else
#define BEE_API
#endif
#define BEE_CALL __stdcall
#define BEE_CALLBACK __stdcall
#else
#define BEE_API
#define BEE_CALL
#define BEE_CALLBACK
#endif

//////////////////////////////////////////////////////////////////////////
//C Standards
//////////////////////////////////////////////////////////////////////////
#if defined(__STDC__)
# define PREDEF_STANDARD_C_1989
# if defined(__STDC_VERSION__)
#  define PREDEF_STANDARD_C_1990
#  if (__STDC_VERSION__ >= 199409L)
#   define PREDEF_STANDARD_C_1994
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define PREDEF_STANDARD_C_1999
#  endif
# endif
#endif

//////////////////////////////////////////////////////////////////////////
// Pre-C89 compilers do not recognize certain keywords. 
// Let the preprocessor remove those keywords for those compilers.
//////////////////////////////////////////////////////////////////////////
#if !defined(PREDEF_STANDARD_C_1989) && !defined(__cplusplus)
# define const
# define volatile
#endif

//////////////////////////////////////////////////////////////////////////
// Define 8-bits Integer, 16-bits Integer,32-bits Integer
// All compliant compilers that support Standard C/C++
// VC++, Borland C++, Turb C++ those who support C89,but undefined __STDC__) 
//////////////////////////////////////////////////////////////////////////
#if defined(__STDC__) || defined(__cplusplus) || defined(_MSC_VER) || defined(__BORLANDC__) ||  defined(__TURBOC__)
#include <limits.h>
// Defined 8 - bit Integer
#if defined(UCHAR_MAX) && (UCHAR_MAX == 0xFF)
typedef char bee_int8_t, *bee_int8_p;
typedef unsigned char bee_uint8_t, *bee_uint8_p;
#ifndef DEFINED_INT8
#define DEFINED_INT8
#endif
#endif
// Defined 16-bits Integer
#if defined(USHRT_MAX) && (USHRT_MAX == 0xFFFF)
typedef short int bee_int16_t, *bee_int16_p;
typedef unsigned short int bee_uint16_t, *bee_uint16_p;
#ifndef DEFINED_INT16
#define DEFINED_INT16
#endif
#elif defined(UINT_MAX) && (UINT_MAX == 0xFFFF)
typedef int bee_int16_t, *bee_int16_p;
typedef unsigned int bee_uint16_t, *bee_uint16_p;
#ifndef DEFINED_INT16
#define DEFINED_INT16
#endif
#endif
// Defined 32-bits Integer
#if defined(UINT_MAX) && (UINT_MAX == 0xFFFFFFFFUL)
typedef int bee_int32_t, *bee_int32_p;
typedef unsigned int bee_uint32_t, *bee_uint32_p;
#ifndef DEFINED_INT32
#define DEFINED_INT32
#endif
#elif defined(ULONG_MAX) && (ULONG_MAX == 0xFFFFFFFFUL)
typedef long int bee_int32_t, *bee_int32_p;
typedef unsigned long int bee_uint32_t, *bee_uint32_p;
#ifndef DEFINED_INT32
#define DEFINED_INT32
#endif
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// Define 64-bits Integer
// Here Only support typical systems 
// such as GNU/Linux Windows UNIX Vxworks  BSD Solaris 
//////////////////////////////////////////////////////////////////////////

// GNU/Linux System 64-bits Integer
#if defined(__GNUC__) || defined(linux) ||defined(__linux)
#if defined (__GLIBC_HAVE_LONG_LONG) || (defined(ULLONG_MAX) && (ULLONG_MAX == 0xFFFFFFFFFFFFFFFFUL)) || defined (PREDEF_STANDARD_C_1999)
typedef long long bee_int64_t, *bee_int64_p;
typedef unsigned long long bee_uint64_t, *bee_uint64_p;
#ifndef DEFINE_INT64
#define DEFINE_INT64
#endif
#endif
#endif

// Windows System 64-bits Integer
#if defined (WIN32) || defined (_WIN32)
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 bee_int64_t, *bee_int64_p;
typedef unsigned __int64 bee_uint64_t, *bee_uint64_p;
#ifndef DEFINE_INT64
#define DEFINE_INT64
#endif
#elif !(defined(unix) || defined(__unix__) || defined(__unix))
typedef unsigned long long bee_int64_t, *bee_int64_p;
typedef signed long long bee_uint64_t, *bee_int64_p;
#ifndef DEFINE_INT64
#define DEFINE_INT64
#endif
#endif
#endif

// UNIX 
#if defined(unix) || defined(__unix__) || defined(__unix)
#define PREDEF_PLATFORM_UNIX
#endif
#if defined(PREDEF_PLATFORM_UNIX)
#include <unistd.h>
#if defined(_XOPEN_VERSION)
#if (_XOPEN_VERSION >= 3)
#define PREDEF_STANDARD_XOPEN_1989
#endif
#if (_XOPEN_VERSION >= 4)
#define PREDEF_STANDARD_XOPEN_1992
#endif
#if (_XOPEN_VERSION >= 4) && defined(_XOPEN_UNIX)
#define PREDEF_STANDARD_XOPEN_1995
#endif
#if (_XOPEN_VERSION >= 500)
#define PREDEF_STANDARD_XOPEN_1998
#endif
#if (_XOPEN_VERSION >= 600)
#define PREDEF_STANDARD_XOPEN_2003
typedef unsigned long long bee_uint64_t, *bee_uint64_p;
typedef signed long long bee_int64_t, *bee_int64_p;
#ifndef DEFINE_INT64
#define DEFINE_INT64
#endif
#endif
#endif
#endif

#ifdef  WIN32
typedef wchar_t  bee_char_t;
typedef wchar_t* bee_char_p;
#else
typedef char     bee_char_t;
typedef char*    bee_char_p;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL     0
#else
#define NULL     ((void *)0)
#endif
#endif

/// 会话句柄类型.
typedef bee_int32_t bee_handle;

/// 错误码.
typedef enum BeeErrorCode {
    kBeeErrorCode_Success = 0,                          //!< 成功.
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
}BeeErrorCode;

/// 平台类型.
typedef enum BeePlatformType {
    kPlatformType_None = 0,             //!< 空类型.
    kPlatformType_PC,                   //!< Windows电脑.
    kPlatformType_Mac,                  //!< 苹果电脑.
    kPlatformType_IPhone,               //!< 苹果手机.
    kPlatformType_IPad,                 //!< 苹果平板.
    kPlatformType_Android_Phone,        //!< Android手机.
    kPlatformType_Android_Pad,          //!< Android平板.
    kPlatformType_Android_TV,           //!< Android电视.
    kPlatformType_Android_Router,       //!< Android路由器.
    kPlatformType_Android_Box           //!< Android盒子.
}BeePlatformType;

/// 网络类型.
typedef enum BeeNetType {
    kNetType_None = 0,                  //!< 空类型.
    kNetType_WireLine,                  //!< 有线网络.
    kNetType_Wifi,                      //!< 无线局域网.
    kNetType_2G,                        //!< 2G网络.
    kNetType_3G,                        //!< 3G网络.
    kNetType_4G                         //!< 4G网络.
}BeeNetType;

/// 日志级别.
typedef enum BeeLogLevel {
    kLogLevel_Fatal = 0,                //!< 致命错误.
    kLogLevel_Error,                    //!< 错误.
    kLogLevel_Warn,                     //!< 警告.
    kLogLevel_Trace,                    //!< 跟踪.
    kLogLevel_Info,                     //!< 信息.
    kLogLevel_Debug,                    //!< 调试.
    kLogLevel_All,                      //!< 所有级别.
    kLogLevel_Count                     //!< 日志级别总数.
}BeeLogLevel;

/// 日志级别字符串.
static const char *BeeLogLevelStr[kLogLevel_Count] = {
    "FT",
    "ER",
    "WA",
    "TR",
    "IF",
    "DT",
    "**"
};

/**
 *  @brief  日志回调类型.
 *  @param  log         日志行.
 */
typedef void(BEE_CALLBACK *BeeLogCallback)(const char *log);

/**
 *  @brief  系统通知回调类型(保留).
 *  @param  handle      会话句柄.
 *  @param  ec1         系统错误码.
 *  @param  ec2         平台相关错误码.
 *  @param  message     错误描述.
 *  @param  opaque      透传参数.
 */
typedef void(BEE_CALLBACK *BeeSystemNotify)(bee_handle handle, BeeErrorCode ec1, bee_int32_t ec2, const char *message, void *opaque);

/**
 *  @brief  系统初始化回调类型.
 *  @param  ec1         系统错误码.
 *  @param  ec2         平台相关错误码.
 *  @param  opaque      透传参数.
 */
typedef void(BEE_CALLBACK *BeeInitCallback)(BeeErrorCode ec1, bee_int32_t ec2, void *opaque);

/**
 *  @brief  打开会话回调类型.
 *  @param  handle      会话句柄.
 *  @param  ec1         系统错误码.
 *  @param  opaque      透传参数.
 */
typedef void(BEE_CALLBACK *BeeOpenCallback)(bee_handle handle, BeeErrorCode ec1, void *opaque);

/// BeeSDK底层系统初始化参数结构.
typedef struct BeeSystemParam {
    BeePlatformType     platform_type;                          //!< 平台类型.
    BeeNetType          net_type;                               //!< 网络类型(保留).
    const char          *app_name;                              //!< APP名称.
    const char          *app_version;                           //!< APP版本.
    const char          *system_info;                           //!< 系统信息.
    const char          *machine_code;                          //!< 设备唯一标识.
    const char          *log_path;                              //!< 日志路径.
    BeeLogLevel         log_level;                              //!< 日志级别.
    bee_int32_t         log_max_line;                           //!< 未分卷时单个日志文件最大行数.
    bee_int32_t         log_volume_count;                       //!< 日志分卷数.
    bee_int32_t         log_volume_size;                        //!< 日志分卷大小.
    bee_uint32_t        session_count;                          //!< 会话数.
    bool                enable_statusd;                         //!< 是否使能实时监控.
    bool                enable_video_encoder_hw_acceleration;   //!< 是否使能编码硬件加速(Android).
    bool                enable_video_decoder_hw_acceleration;   //!< 是否使能解码硬件加速(Android).
    bool                enable_custom_audio_source;             //!< 是否使用自定义音频源.
}BeeSystemParam;

/// JSON对象拼接工具.
#define BEE_TO_STR(s) #s

#define BEE_OBJ_BEGIN "{"
#define BEE_OBJ_END "}"

#define BEE_STR_OBJ_BEGIN(key) "\""#key"\":\""
#define BEE_STR_OBJ_CONTINUE "\","
#define BEE_STR_OBJ_END "\""

#define BEE_VAL_OBJ_BEGIN(key) "\""#key"\":"
#define BEE_VAL_OBJ_CONTINUE ","
#define BEE_VAL_OBJ_END ""

#define BEE_COM_OBJ_BEGIN(key) "\""#key"\":{"
#define BEE_COM_OBJ_CONTINUE(key) "},"
#define BEE_COM_OBJ_END(key) "}"

#endif
