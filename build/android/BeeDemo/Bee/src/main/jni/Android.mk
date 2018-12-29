LOCAL_PATH:=$(call my-dir)
MEDIA_SDK_ROOT  := $(LOCAL_PATH)/../../../../../../..
ENABLE_BREAKPAD := false

include $(CLEAR_VARS)
LOCAL_MODULE    := boost_date_time
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/boost/android/lib/$(TARGET_ARCH_ABI)/libboost_date_time.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := boost_regex 
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/boost/android/lib/$(TARGET_ARCH_ABI)/libboost_regex.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := boost_system
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/boost/android/lib/$(TARGET_ARCH_ABI)/libboost_system.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lua
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/lua/android/lib/$(TARGET_ARCH_ABI)/liblua.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := crypto
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/openssl/android/lib/$(TARGET_ARCH_ABI)/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ssl
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/openssl/android/lib/$(TARGET_ARCH_ABI)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := zlib
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/libwebsockets/android/lib/$(TARGET_ARCH_ABI)/libz.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := lws
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/libwebsockets/android/lib/$(TARGET_ARCH_ABI)/libwebsockets.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := webrtc
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/webrtc/android/lib/$(TARGET_ARCH_ABI)/libwebrtc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := breakpad_client
LOCAL_SRC_FILES := $(MEDIA_SDK_ROOT)/third_party/breakpad/android/lib/$(TARGET_ARCH_ABI)/libbreakpad_client.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := bee

SOURCE_PATH:= $(MEDIA_SDK_ROOT)/src

#Source
#api
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/api/bee/base/bee.cpp\
				$(SOURCE_PATH)/api/bee/base/bee_service.cpp

#media
LOCAL_SRC_FILES += \
                $(SOURCE_PATH)/api/bee/media/audio_source.cpp\
				$(SOURCE_PATH)/api/bee/media/audio_source_default.cpp\
				$(SOURCE_PATH)/api/bee/media/video_frame.cpp\
				$(SOURCE_PATH)/api/bee/media/video_renderer.cpp\
				$(SOURCE_PATH)/api/bee/media/video_source.cpp

#internal
LOCAL_SRC_FILES += \
                $(SOURCE_PATH)/internal/audio_source_internal.cpp\
                $(SOURCE_PATH)/internal/video_frame_internal.cpp\
                $(SOURCE_PATH)/internal/video_renderer_internal.cpp\
                $(SOURCE_PATH)/internal/video_source_internal.cpp

#server
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/server/ca/ca_client.cpp\
				$(SOURCE_PATH)/server/statusd/statusd_client.cpp

#http
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/http/http_request.cpp\
				$(SOURCE_PATH)/http/http_response.cpp

#log
LOCAL_SRC_FILES += \
                $(SOURCE_PATH)/log/logger.cpp\
                $(SOURCE_PATH)/log/logger_impl.cpp
				
#lua
LOCAL_SRC_FILES += \
               $(SOURCE_PATH)/lua/module/cjson/fpconv.c\
               $(SOURCE_PATH)/lua/module/cjson/lua_cjson.c\
               $(SOURCE_PATH)/lua/module/cjson/lua_extensions.c\
               $(SOURCE_PATH)/lua/module/cjson/strbuf.c\
               $(SOURCE_PATH)/lua/module/lua_crypto_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_data_promise.cpp\
               $(SOURCE_PATH)/lua/module/lua_http_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_http_reader.cpp\
               $(SOURCE_PATH)/lua/module/lua_http_session.cpp\
               $(SOURCE_PATH)/lua/module/lua_iobuffer_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_lws_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_lws_service.cpp\
               $(SOURCE_PATH)/lua/module/lua_lws_session.cpp\
               $(SOURCE_PATH)/lua/module/lua_timer.cpp\
               $(SOURCE_PATH)/lua/module/lua_timer_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_video_cache.cpp\
               $(SOURCE_PATH)/lua/module/lua_webrtc_module.cpp\
               $(SOURCE_PATH)/lua/module/lua_webrtc_peer_connection.cpp\
               $(SOURCE_PATH)/lua/module/lua_webrtc_service.cpp\
               $(SOURCE_PATH)/lua/module/lua_webrtc_stats_log_observer.cpp\
               $(SOURCE_PATH)/lua/module/lua_webrtc_video_sink.cpp\
               $(SOURCE_PATH)/lua/lua_default_http_session.cpp\
               $(SOURCE_PATH)/lua/lua_engine.cpp

#network
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/network/http_session.cpp\
				$(SOURCE_PATH)/network/http_state_machine.cpp\
				$(SOURCE_PATH)/network/io_service.cpp\
				$(SOURCE_PATH)/network/tcp_state_machine.cpp

#platform/android
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/platform/android/android_BeeService.cpp\
				$(SOURCE_PATH)/platform/android/com_sohu_tv_bee_BeeSDK.cpp\
				$(SOURCE_PATH)/platform/android/com_sohu_tv_bee_BeeSDKService.cpp\
                $(SOURCE_PATH)/platform/android/com_sohu_tv_bee_BeeVideoRender.cpp\
                $(SOURCE_PATH)/platform/android/com_sohu_tv_bee_BeeAudioSource.cpp\
                $(SOURCE_PATH)/platform/android/com_sohu_tv_bee_BeeVideoSource.cpp\
                $(SOURCE_PATH)/platform/android/bee_video_source_jni_adapter.cpp\
                $(SOURCE_PATH)/platform/android/bee_video_renderer_jni_adapter.cpp\
				$(SOURCE_PATH)/platform/android/androidhistogram_jni.cc\
                $(SOURCE_PATH)/platform/android/androidmediadecoder_jni.cc\
                $(SOURCE_PATH)/platform/android/androidmediaencoder_jni.cc\
                $(SOURCE_PATH)/platform/android/androidmetrics_jni.cc\
                $(SOURCE_PATH)/platform/android/androidnetworkmonitor_jni.cc\
                $(SOURCE_PATH)/platform/android/androidvideotracksource.cc\
                $(SOURCE_PATH)/platform/android/androidvideotracksource_jni.cc\
                $(SOURCE_PATH)/platform/android/classreferenceholder.cc\
                $(SOURCE_PATH)/platform/android/jni_helpers.cc\
                $(SOURCE_PATH)/platform/android/jni_onload.cc\
                $(SOURCE_PATH)/platform/android/peerconnection_jni.cc\
                $(SOURCE_PATH)/platform/android/native_handle_impl.cc\
                $(SOURCE_PATH)/platform/android/surfacetexturehelper_jni.cc\
                $(SOURCE_PATH)/platform/android/adaptedvideotracksource.cc\
                $(SOURCE_PATH)/platform/android/videobroadcaster.cc\
                $(SOURCE_PATH)/platform/android/videosourcebase.cc

#service
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/service/bee_promise.cpp\
				$(SOURCE_PATH)/service/bee_entrance.cpp
				
#session
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/session/bee_session.cpp\
				$(SOURCE_PATH)/session/session_manager.cpp
				
#state
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/state/async_state_machine.cpp\
				$(SOURCE_PATH)/state/simple_state_machine.cpp\
				$(SOURCE_PATH)/state/state.cpp
				
#utility
LOCAL_SRC_FILES += \
				$(SOURCE_PATH)/utility/buffer_pool.cpp\
				$(SOURCE_PATH)/utility/crypto.cpp\
				$(SOURCE_PATH)/utility/json/json_reader.cpp\
				$(SOURCE_PATH)/utility/json/json_value.cpp\
				$(SOURCE_PATH)/utility/json/json_writer.cpp\
				$(SOURCE_PATH)/utility/file_util.cpp

#Include file path
LOCAL_C_INCLUDES := \
                $(NDK_ROOT)/platforms/$(APP_PLATFORM)/arch-$(TARGET_ARCH)/usr/include\
				$(MEDIA_SDK_ROOT)/third_party/boost/android/include\
				$(MEDIA_SDK_ROOT)/third_party/openssl/android/include\
				$(MEDIA_SDK_ROOT)/third_party/lua/android/include\
				$(MEDIA_SDK_ROOT)/third_party/libwebsockets/android/include\
				$(MEDIA_SDK_ROOT)/third_party/webrtc/android/include\
				$(MEDIA_SDK_ROOT)/third_party/webrtc/android/include\third_party\libyuv\include\
				$(MEDIA_SDK_ROOT)/src\
				$(MEDIA_SDK_ROOT)/src/api

ifeq ($(ENABLE_BREAKPAD),true)
LOCAL_C_INCLUDES += $(MEDIA_SDK_ROOT)/third_party/breakpad/android/include
LOCAL_C_INCLUDES += $(MEDIA_SDK_ROOT)/third_party/breakpad/android/include/breakpad
endif

#Static libraries.
LOCAL_STATIC_LIBRARIES := \
                boost_date_time\
				boost_regex\
				boost_system\
				lua\
				webrtc\
				lws\
				zlib\
				ssl\
				crypto

ifeq ($(ENABLE_BREAKPAD),true)
LOCAL_STATIC_LIBRARIES += breakpad_client
endif

LOCAL_CPP_FEATURES  += exceptions
LOCAL_CFLAGS    := -DANDROID -g -fPIC
#-fsanitize=leak not supported by ndk yet.
#LOCAL_CFLAGS   += -fsanitize=address -fno-omit-frame-pointer

LOCAL_CPPFLAGS  := $(LOCAL_CFLAGS) -std=c++11
LOCAL_CPPFLAGS  += -DV8_DEPRECATION_WARNINGS
LOCAL_CPPFLAGS  += -DUSE_AURA=1
LOCAL_CPPFLAGS  += -DNO_TCMALLOC
LOCAL_CPPFLAGS  += -DFULL_SAFE_BROWSING
LOCAL_CPPFLAGS  += -DSAFE_BROWSING_CSD
LOCAL_CPPFLAGS  += -DSAFE_BROWSING_DB_LOCAL
LOCAL_CPPFLAGS  += -DCHROMIUM_BUILD
LOCAL_CPPFLAGS  += -DPSAPI_VERSION=1
LOCAL_CPPFLAGS  += -DENABLE_MEDIA_ROUTER=1
LOCAL_CPPFLAGS  += -DFIELDTRIAL_TESTING_ENABLED
LOCAL_CPPFLAGS  += -DCOMPONENT_BUILD
LOCAL_CPPFLAGS  += -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS
LOCAL_CPPFLAGS  += -DNOMINMAX
LOCAL_CPPFLAGS  += -DDYNAMIC_ANNOTATIONS_ENABLED=1
LOCAL_CPPFLAGS  += -DWTF_USE_DYNAMIC_ANNOTATIONS=1
LOCAL_CPPFLAGS  += -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE
LOCAL_CPPFLAGS  += -DEXPAT_RELATIVE_PATH
LOCAL_CPPFLAGS  += -DHAVE_SCTP
LOCAL_CPPFLAGS  += -DWEBRTC_POSIX
LOCAL_CPPFLAGS  += -D__STDC_CONSTANT_MACROS
LOCAL_CPPFLAGS  += -DTEST_CA

ifeq ($(ENABLE_BREAKPAD),true)
$(info "Enable Breakpad")
LOCAL_CPPFLAGS	+= -DENABLE_BREAKPAD
else
$(info "Disable Breakpad")
endif

LOCAL_LDFLAGS   += -Wl,-v
#-fsanitize=leak not supported by ndk yet.
#LOCAL_LDFLAGS  += -fsanitize=address
LOCAL_LDLIBS    := -llog -lOpenSLES -lGLESv2
LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

include $(BUILD_SHARED_LIBRARY)
