#import "bee_sdk_service.h"
#include "bee.h"
#include "bee_service.h"

using namespace bee;

#pragma mark - BeeServiceOcAdapter

@class BeeSDKService;

class BeeServiceOcAdapter : public BeeService {
public:
    BeeServiceOcAdapter(bee_int32_t svcCode, BeeSDKService *ocService) : BeeService(svcCode), _ocService(ocService) {}
    virtual ~BeeServiceOcAdapter() {}
    
    void handle_data(const std::string &data) {
        if (_ocService != nil) {
            NSString *ocData = [NSString stringWithUTF8String:data.c_str()];
            [_ocService handleData:ocData];
        }
    }
    
private:
    __weak BeeSDKService *_ocService;
};

#pragma mark - BeeSDKService

@implementation BeeSDKService {
    std::shared_ptr<BeeServiceOcAdapter> _internalService;
    bee_handle _handle;
    BOOL _registered;
}

- (instancetype)initWithSvcCode:(bee_int32_t)svcCode {
    if (self = [super init]) {
        _internalService.reset(new BeeServiceOcAdapter(svcCode, self));
        _handle = -1;
        _registered = NO;
    }
    return self;
}

- (BeeErrorCode)Register:(bee_handle)handle {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (_internalService == NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
        
        if (handle == -1) {
            ret = kBeeErrorCode_Session_Not_Opened;
            break;
        }
        
        ret = _internalService->reg(handle);
        if (ret != kBeeErrorCode_Success) {
            break;
        }
        
        _registered = YES;
    } while (false);
    return ret;
}

- (BeeErrorCode)unRegister {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (!_registered) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
        
        if (_internalService == NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
        
        ret = _internalService->unreg();
        if (ret != kBeeErrorCode_Success) {
            NSLog(@"[WA] Unreg service %d failed with error %d", _internalService->get_svc_code(), ret);
        }
        
        _registered = NO;
    } while (false);
    return ret;
}

- (BOOL)isRegistered {
    return _registered;
}

- (BeeErrorCode)execute:(NSString*)command args:(NSString*)args timeout:(bee_int32_t)timeout {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (_internalService == NULL) {
            ret = kBeeErrorCode_Invalid_State;
            break;
        }
        
        if (command == nil || args == nil) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        const char *cCmd = [command UTF8String];
        const char *cArgs = [args UTF8String];
        if (cCmd == NULL || cArgs == NULL) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }
        
        ret = _internalService->execute(std::string(cCmd), std::string(cArgs), timeout);
    } while (false);
    return ret;
}

- (void)handleData:(NSString*)data; {
    //Do nothing.
}

@end
