/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MEDIA_BASE_VIDEOSOURCEBASE_H_
#define WEBRTC_MEDIA_BASE_VIDEOSOURCEBASE_H_

#include <vector>

#include "webrtc/api/video/video_frame.h"
#include "webrtc/base/thread_checker.h"
#include "webrtc/media/base/videosourceinterface.h"

namespace bee {

// VideoSourceBase is not thread safe.
class VideoSourceBase : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
 public:
  VideoSourceBase();
  void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                       const rtc::VideoSinkWants& wants) override;
  void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

 protected:
  struct SinkPair {
    SinkPair(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, rtc::VideoSinkWants wants)
        : sink(sink), wants(wants) {}
    rtc::VideoSinkInterface<webrtc::VideoFrame>* sink;
      rtc::VideoSinkWants wants;
  };
  SinkPair* FindSinkPair(const rtc::VideoSinkInterface<webrtc::VideoFrame>* sink);

  const std::vector<SinkPair>& sink_pairs() const { return sinks_; }
    rtc::ThreadChecker thread_checker_;

 private:
  std::vector<SinkPair> sinks_;
};

}  // namespace rtc

#endif  // WEBRTC_MEDIA_BASE_VIDEOSOURCEBASE_H_
