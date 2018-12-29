#ifndef __LUA_WEBRTC_VIDEO_SINK_H__
#define __LUA_WEBRTC_VIDEO_SINK_H__

#include "utility/common.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/video/video_frame.h"
#include "comLib/SafeQueue.h"
#include "comLib/Thread.h"
#include "bee/base/bee_define.h"
#include "bee/media/video_frame.h"
#include "log/logger.h"

namespace bee {

////////////////////////////////////LuaWebrtcVideoSink//////////////////////////////////////
class LuaRtcPeerConnection;
class VideoRenderer;
class LuaWebrtcVideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    LuaWebrtcVideoSink(webrtc::VideoTrackInterface* track_to_render, std::shared_ptr<LuaRtcPeerConnection> rtc_peer_connection);
    virtual ~LuaWebrtcVideoSink();

public:
    virtual void start(VideoRenderer *video_renderer);
    virtual void stop();
    virtual void OnFrame(const webrtc::VideoFrame &frame) override;

protected:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
    std::weak_ptr<LuaRtcPeerConnection> rtc_peer_connection_;
    VideoFrame bee_video_frame_;
    VideoRenderer *video_renderer_ = NULL;
    Logger logger_;
    bool first_frame_ = true;
};

} // namespace bee

#endif
