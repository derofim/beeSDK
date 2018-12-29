#include "bee.h"
#include "bee_sink.h"
#import "bee_sdk.h"

using namespace bee;

#pragma mark - BeeSDKParam

@implementation BeeSDKParam

@synthesize platformType = _platformType;
@synthesize appName = _appName;
@synthesize appVersion = _appVersion;
@synthesize systemInfo = _systemInfo;
@synthesize machineCode = _machineCode;
@synthesize logPath = _logPath;
@synthesize logLevel = _logLevel;
@synthesize logMaxLine = _logMaxLine;
@synthesize logVolumeCount = _logVolumeCount;
@synthesize logVolumeSize = _logVolumeSize;
@synthesize sessionCount = _sessionCount;
@synthesize enableStatusd  = _enableStatusd;

- (instancetype)initWithParam:(BeePlatformType)platformType
                      appName:(NSString*)appName
                   appVersion:(NSString*)appVersion
                   systemInfo:(NSString*)systemInfo
                  machineCode:(NSString*)machineCode
                      logPath:(NSString*)logPath
                     logLevel:(BeeLogLevel)logLevel
                   logMaxLine:(bee_int32_t)logMaxLine
               logVolumeCount:(bee_int32_t)logVolumeCount
                logVolumeSize:(bee_int32_t)logVolumeSize
                 sessionCount:(bee_uint32_t)sessionCount
                enableStatusd:(bool)enableStatusd {
    if (self = [super init]) {
        _platformType = platformType;
        _appName = appName;
        _appVersion = appVersion;
        _systemInfo = systemInfo;
        _machineCode = machineCode;
        _logPath = logPath;
        _logLevel = logLevel;
        _logMaxLine = logMaxLine;
        _logVolumeCount = logVolumeCount;
        _logVolumeSize = logVolumeSize;
        _sessionCount = sessionCount;
        _enableStatusd = enableStatusd;
    }
    return self;
}

@end

#pragma mark - BeeSDKCapability

@implementation BeeSDKCapability

@synthesize svcCode = _svcCode;
@synthesize description = _description;

- (instancetype)initWithParam:(bee_int32_t)svcCode description:(NSString*)description {
    if (self = [super init]) {
        _svcCode = svcCode;
        _description = description;
    }
    return self;
}
@end

#pragma mark - BeeSDKSinkOcAdapter

class BeeSDKSinkOcAdapter : public BeeSink {
public:
    virtual void on_log(const char *log) {
        if (_sink != nil && log != NULL) {
            NSString *nsLog = [NSString stringWithUTF8String:log];
            [_sink onLog:nsLog];
        }
    }
    
    virtual void on_notify(BeeErrorCode ec1, bee_int32_t ec2) {
        if (_sink != nil) {
            [_sink onNotify:ec1 ec2:ec2];
        }
    }
    
    void setOcSink(id<BeeSDKSink> sink) {
        _sink = sink;
    }
    
private:
    __weak id<BeeSDKSink> _sink;
};

#pragma mark - BeeSDK

@implementation BeeSDK {
    __weak id<BeeSDKSink> _beeSDKSink;
    std::shared_ptr<BeeSDKSinkOcAdapter> _sinkAdapter;
    dispatch_queue_t _dispatchQueue;
    BOOL _initialized;
}

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    static BeeSDK *sharedInstance = nil;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    if (self = [super init]) {
        _sinkAdapter.reset(new BeeSDKSinkOcAdapter);
        _dispatchQueue = dispatch_queue_create("BeeSDK", NULL);
        _initialized = NO;
    }
    return self;
}

- (void)initialize:(BeeSDKParam*)param timeout:(int)timeout sink:(id<BeeSDKSink>)beeSDKSink handler:(InitHandler)handler {
    dispatch_async([self getDispatchQueue], ^{
        [self initializeInternal:param timeout:timeout sink:beeSDKSink handler:handler];
    });
}

- (void)uninitialize:(UnInitHandler)handler {
    dispatch_async([self getDispatchQueue], ^{
        [self uninitializeInternal:handler];
    });
}

- (void)openSession:(OpenSessionHandler)handler {
    dispatch_async([self getDispatchQueue], ^{
        [self openSessionInternal:handler];
    });
}

- (void)closeSession:(bee_handle)handle handler:(CloseSessionHandler)handler {
    dispatch_async([self getDispatchQueue], ^{
        [self closeSessionInternal:handle handler:handler];
    });
}

- (NSString*)errorToString:(BeeErrorCode)error {
    return @"";
}

- (dispatch_queue_t)getDispatchQueue {
    return _dispatchQueue;
}

#pragma mark - Internal methods.

- (void)initializeInternal:(BeeSDKParam*)param timeout:(int)timeout sink:(id<BeeSDKSink>)beeSDKSink handler:(InitHandler)handler {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (_initialized) {
            break;
        }
        
        BeeSystemParam beeSysParam;
        memset(&beeSysParam, 0, sizeof(beeSysParam));
        
        ret = [self setupBeeSysParam:param outParam:&beeSysParam];
        if (ret != kBeeErrorCode_Success) {
            break;
        }
        
        _beeSDKSink = beeSDKSink;
        if (_sinkAdapter != NULL) {
            _sinkAdapter->setOcSink(_beeSDKSink);
        }
        
        bee_int32_t ec2 = 0;
        ret = Bee::instance()->initialize(beeSysParam, NULL, timeout, ec2);
        if (ret != kBeeErrorCode_Success) {
            break;
        }
        
        _initialized = YES;
    } while (false);
    
    if (handler != nil) {
        handler(ret);
    }
}

- (void)uninitializeInternal:(UnInitHandler)handler {
    BeeErrorCode ret = kBeeErrorCode_Success;
    if (_initialized) {
        ret = Bee::instance()->uninitialize();
        _initialized = NO;
    }
    if (handler != nil) {
        handler(ret);
    }
}

- (void)openSessionInternal:(OpenSessionHandler)handler {
    //User must supply handler to get result.
    if (handler == nil) {
        return;
    }
    
    bee_handle handle = -1;
    std::vector<BeeCapability> capabilitiesInternal;
    BeeErrorCode ret = Bee::instance()->open_session(handle, capabilitiesInternal);
    
    NSMutableArray<BeeSDKCapability*> *capabilities = nil;
    if (!capabilitiesInternal.empty()) {
        capabilities = [[NSMutableArray alloc] initWithCapacity:capabilitiesInternal.size()];
        for(BeeCapability capInternal : capabilitiesInternal) {
            BeeSDKCapability *cap = [[BeeSDKCapability alloc] init];
            cap.svcCode = capInternal.svc_code;
            cap.description = [NSString stringWithUTF8String:capInternal.description.c_str()];
            [capabilities addObject:cap];
        }
    }
    
    handler(ret, handle, capabilities);
}

- (void)closeSessionInternal:(bee_handle)handle handler:(CloseSessionHandler)handler{
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (handle == -1) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        ret = Bee::instance()->close_session(handle);
    } while (false);
    
    if (handler != nil) {
        handler(ret);
    }
}

#pragma mark - Private methods.

- (BeeErrorCode)setupBeeSysParam:(BeeSDKParam*)inParam outParam:(BeeSystemParam*)outParam {
    if (inParam == nil || outParam == nil) {
        return kBeeErrorCode_Invalid_Param;
    }
    
    outParam->platform_type     = inParam.platformType;
    outParam->app_name          = [inParam.appName UTF8String];
    outParam->app_version       = [inParam.appVersion UTF8String];
    outParam->system_info       = [inParam.systemInfo UTF8String];
    outParam->machine_code      = [inParam.machineCode UTF8String];
    outParam->log_path          = [inParam.logPath UTF8String];
    outParam->log_level         = inParam.logLevel;
    outParam->log_max_line      = inParam.logMaxLine;
    outParam->log_volume_count  = inParam.logVolumeCount;
    outParam->log_volume_size   = inParam.logVolumeSize;
    outParam->session_count     = inParam.sessionCount;
    outParam->enable_statusd    = inParam.enableStatusd;
    
    if (outParam->session_count == -1) {
        outParam->session_count = 16;
    }
    
    return kBeeErrorCode_Success;
}

@end

