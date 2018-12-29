#include "bee/win/win_video_source_cam.h"
#include "internal/video_source_internal.h"
#include "lua/module/lua_webrtc_service.h"
#include "webrtc/media/base/videocapturer.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/test/fakeconstraints.h"

namespace bee {

WinVideoSourceCam::WinVideoSourceCam(    
    bee_int32_t width,
    bee_int32_t height,
    bee_int32_t fps,
    bee_int32_t camera_index)
    : VideoSource(width, height, fps, false),
      camera_index_(camera_index), 
      opened_(false) {

}

WinVideoSourceCam::~WinVideoSourceCam() {

}

BeeErrorCode WinVideoSourceCam::open() {
    BeeErrorCode ret = kBeeErrorCode_Success;
    do {
        if (opened_) {
            break;
        }

        std::vector<std::string> device_names;
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
        if (info == NULL) {
            ret = kBeeErrorCode_Create_Device_Info_Failed;
            break;
        }

        uint32_t num_devices = info->NumberOfDevices();
        if (num_devices == 0) {
            ret = kBeeErrorCode_No_Camera;
            break;
        }

        if (camera_index_ >= num_devices) {
            ret = kBeeErrorCode_Invalid_Param;
            break;
        }

        for (uint32_t i = 0; i < num_devices; ++i) {
            const uint32_t kSize = 512;
            char name[kSize] = { 0 };
            char id[kSize] = { 0 };
            if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
                device_names.push_back(name);
            }
        }

        cricket::WebRtcVideoDeviceCapturerFactory factory;
        std::unique_ptr<cricket::VideoCapturer> capturer = factory.Create(cricket::Device(device_names[camera_index_], 0));
        if (capturer == NULL) {
            ret = kBeeErrorCode_Open_Camera_Failed;
            break;
        }

        LuaWebrtcService::FactoryPtr peerconnection_factory = LuaWebrtcService::peer_connection_factory();
        if (peerconnection_factory == NULL) {
            ret = kBeeErrorCode_Null_Pointer;
            break;
        }

        webrtc::FakeConstraints video_constraints;
        video_constraints.AddMandatory(
            webrtc::MediaConstraintsInterface::kMaxFrameRate, fps_);
        video_constraints.AddMandatory(
            webrtc::MediaConstraintsInterface::kMaxWidth, width_);
        video_constraints.AddMandatory(
            webrtc::MediaConstraintsInterface::kMaxHeight, height_);

        video_source_internal_->rtc_video_track_source_ = peerconnection_factory->CreateVideoSource(capturer.release(), &video_constraints);
        opened_ = true;
    } while (0);
    return ret;
}

} // namespace bee
