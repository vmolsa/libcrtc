
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

#ifndef CRTC_VIDEOSOURCE_H
#define CRTC_VIDEOSOURCE_H

#include "crtc.h"
#include "mediastream.h"
#include "mediastreamtrack.h"
#include "worker.h"
#include "imagebuffer.h"
#include "videocapturer.h"

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/modules/audio_device/include/audio_device.h"
#include "webrtc/base/sigslot.h"

namespace crtc {
  class VideoSourceInternal : public VideoSource, public MediaStreamInternal, public sigslot::has_slots<> {
      friend class Let<VideoSourceInternal>;
      friend class VideoSource;

    public:
      std::string Id() const override;
      void AddTrack(const Let<MediaStreamTrack> &track) override;
      void RemoveTrack(const Let<MediaStreamTrack> &track) override;
      Let<MediaStreamTrack> GetTrackById(const std::string &id) const override;
      MediaStreamTracks GetAudioTracks() const override;
      MediaStreamTracks GetVideoTracks() const override;
      Let<MediaStream> Clone() override;

      VideoCapturer* GetCapturer() const;

      bool IsRunning() const override;
      void Stop() override;
      int Width() const override;
      int Height() const override;
      float Fps() const override;
      void Write(const Let<ImageBuffer> &frame, ErrorCallback callback = ErrorCallback()) override;

    private:
      void OnStateChange(cricket::VideoCapturer* capturer, cricket::CaptureState capture_state);
      void OnDrain();

      static volatile int counter;
      static Let<WorkerInternal> worker;
      static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
      static rtc::scoped_refptr<webrtc::AudioDeviceModule> audio;

    protected:
      explicit VideoSourceInternal(webrtc::MediaStreamInterface *stream = nullptr);
      ~VideoSourceInternal() override;

      Let<Event> _event;
      VideoCapturer* _capturer;
  };
};

#endif