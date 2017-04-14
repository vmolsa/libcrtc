
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

#ifndef CRTC_AUDIOSINK_H
#define CRTC_AUDIOSINK_H

#include "crtc.h"
#include "mediastream.h"
#include "mediastreamtrack.h"
#include "audiobuffer.h"

namespace crtc {
  class AudioSinkInternal : public AudioSink, public MediaStreamTrackInternal, public webrtc::AudioTrackSinkInterface {
      friend class Let<AudioSinkInternal>;
      friend class AudioSink;

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
      explicit AudioSinkInternal(const Let<MediaStreamTrackInternal> &track, 
                                 const rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track);

      ~AudioSinkInternal() override;

      void OnEnded() override;
      void OnData(const void* audio_data,
                  int bits_per_sample,
                  int sample_rate,
                  size_t number_of_channels,
                  size_t number_of_frames) override;

      Let<Event> _event;
      rtc::scoped_refptr<webrtc::AudioTrackInterface> _audio_track;
  };  
};

#endif