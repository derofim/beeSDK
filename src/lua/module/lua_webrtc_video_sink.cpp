#include "lua_webrtc_video_sink.h"
#include "lua_webrtc_peer_connection.h"
#include "internal/video_frame_internal.h"
#include "bee/media/video_renderer.h"

namespace bee {

LuaWebrtcVideoSink::LuaWebrtcVideoSink(webrtc::VideoTrackInterface* track_to_render, std::shared_ptr<LuaRtcPeerConnection> rtc_peer_connection)
    : rendered_track_(track_to_render),
      rtc_peer_connection_(rtc_peer_connection),
      logger_("LuaWebrtcVideoSink") {
    rtc::VideoSinkWants want;
    want.rotation_applied = true;
    rendered_track_->AddOrUpdateSink(this, want);
    logger_.Debug("LuaWebrtcVideoSink Created %x.\n", this);
    memset(&bee_video_frame_, 0, sizeof(VideoFrame));
}

LuaWebrtcVideoSink::~LuaWebrtcVideoSink() {
    rendered_track_->RemoveSink(this);
    logger_.Debug("LuaWebrtcVideoSink Deleted %x.\n", this);
}

void LuaWebrtcVideoSink::start(VideoRenderer *video_renderer) {
    video_renderer_ = video_renderer;
}

void LuaWebrtcVideoSink::stop() {
    video_renderer_ = NULL; //Not release here.
}

void LuaWebrtcVideoSink::OnFrame(const webrtc::VideoFrame &frame) {
    if (first_frame_) {
        first_frame_ = false;
        logger_.Info("First frame.\n");
    }

    if (video_renderer_ == NULL) {
        return;
    }

    bee_video_frame_.width_ = (bee_int32_t)frame.width();
    bee_video_frame_.height_ = (bee_int32_t)frame.height();
    bee_video_frame_.rotation_ = (bee_int32_t)frame.rotation();
    bee_video_frame_.timestamp_ = (bee_int64_t)frame.timestamp_us();

    bee_video_frame_.video_frame_internal_.reset(new VideoFrameInternal(frame));

#ifdef ANDROID
    //Only support YUV(I420) format.
    if (frame.video_frame_buffer() != NULL) {
        bee_video_frame_.y_data_ = (bee_uint8_t*)frame.video_frame_buffer()->DataY();
        bee_video_frame_.u_data_ = (bee_uint8_t*)frame.video_frame_buffer()->DataU();
        bee_video_frame_.v_data_ = (bee_uint8_t*)frame.video_frame_buffer()->DataV();
        bee_video_frame_.y_stride_ = (bee_int32_t)frame.video_frame_buffer()->StrideY();
        bee_video_frame_.u_stride_ = (bee_int32_t)frame.video_frame_buffer()->StrideU();
        bee_video_frame_.v_stride_ = (bee_int32_t)frame.video_frame_buffer()->StrideV();
    }
#else
    //Only support YUV(I420/I444) format.
    if (frame.video_frame_buffer() != NULL &&
        (frame.video_frame_buffer()->type() == webrtc::VideoFrameBuffer::Type::kI420 ||
        frame.video_frame_buffer()->type() == webrtc::VideoFrameBuffer::Type::kI444)) {
        webrtc::PlanarYuvBuffer *yuv_buffer = (webrtc::PlanarYuvBuffer*)frame.video_frame_buffer().get();
        if (yuv_buffer != NULL) {
            bee_video_frame_.y_data_ = (bee_uint8_t*)yuv_buffer->DataY();
            bee_video_frame_.u_data_ = (bee_uint8_t*)yuv_buffer->DataU();
            bee_video_frame_.v_data_ = (bee_uint8_t*)yuv_buffer->DataV();
            bee_video_frame_.y_stride_ = (bee_int32_t)yuv_buffer->StrideY();
            bee_video_frame_.u_stride_ = (bee_int32_t)yuv_buffer->StrideU();
            bee_video_frame_.v_stride_ = (bee_int32_t)yuv_buffer->StrideV();
        }
    }
#endif

    //Internal frame for compatible with webrtc renderer.
    video_renderer_->on_frame(bee_video_frame_);
    bee_video_frame_.video_frame_internal_.reset();
}

} // namespace bee
