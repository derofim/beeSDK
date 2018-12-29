#import "bee/ios/bee_audio_source.h"
#include "bee/media/audio_source_default.h"

@implementation BeeAudioSource

@synthesize internalAudioSource = _internalAudioSource;

- (instancetype)initWithParam:(BOOL)levelControl
                   echoCancel:(BOOL)echoCancel
                  gainControl:(BOOL)gainControl
               highPassFilter:(BOOL)highPassFilter
             noiceSuppression:(BOOL)noiceSuppression {
    if (self = [super init]) {
        _internalAudioSource.reset(new bee::AudioSourceDefault(levelControl?true:false,
                                                               echoCancel?true:false,
                                                               gainControl?true:false,
                                                               highPassFilter?true:false,
                                                               noiceSuppression?true:false));
    }
    
    return self;
}

- (BeeErrorCode)open {
    if (_internalAudioSource != NULL) {
        return _internalAudioSource->open();
    } else {
        return kBeeErrorCode_Invalid_State;
    }
}

@end

