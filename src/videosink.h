
/*
* The MIT License (MIT)
*
* Copyright (c) 2017 vmolsa <ville.molsa@gmail.com> (http://github.com/vmolsa)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#ifndef CRTC_VIDEOSINK_H
#define CRTC_VIDEOSINK_H

#include "crtc.h"
#include "mediastream.h"
#include "mediastreamtrack.h"

#include "webrtc/media/base/videosinkinterface.h"
#include "webrtc/video_frame.h"

namespace crtc {  
  class VideoSinkInternal : public VideoSink, public MediaStreamTrackInternal, public rtc::VideoSinkInterface<webrtc::VideoFrame> {
      friend class Let<VideoSinkInternal>;
      friend class VideoSink;

    public:
      bool IsRunning() const override;
      void Stop() override;

      bool Enabled() const override;
      bool Muted() const override;
      bool Remote() const override;
      std::string Id() const override;
      MediaStreamTrack::Type Kind() const override;
      MediaStreamTrack::State ReadyState() const override;
      Let<MediaStreamTrack> Clone() override;
 
    protected:      
      explicit VideoSinkInternal(const Let<MediaStreamTrackInternal> &track, 
                                 const rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track);

      ~VideoSinkInternal() override;

      void OnEnded() override;
      void OnFrame(const webrtc::VideoFrame& frame) override;

      Let<Event> _event;
      rtc::scoped_refptr<webrtc::VideoTrackInterface> _video_track;
  };
};

#endif