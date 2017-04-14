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

#ifndef CRTC_AUDIOSOURCE_H
#define CRTC_AUDIOSOURCE_H

#include "crtc.h"
#include "mediastream.h"

namespace crtc { 
  class AudioSourceInternal : public AudioSource, public MediaStreamInternal {
      friend class Let<AudioSourceInternal>;
      friend class AudioSource;
    public:
      bool IsRunning() const override;
      void Stop() override;

      void Write(const Let<AudioBuffer> &buffer, ErrorCallback callback = ErrorCallback()) override;

      std::string Id() const override;
      void AddTrack(const Let<MediaStreamTrack> &track) override;
      void RemoveTrack(const Let<MediaStreamTrack> &track) override;
      Let<MediaStreamTrack> GetTrackById(const std::string &id) const override;
      MediaStreamTracks GetAudioTracks() const override;
      MediaStreamTracks GetVideoTracks() const override;
      Let<MediaStream> Clone() override;

    protected:
      explicit AudioSourceInternal();
      ~AudioSourceInternal() override;
  };
};

#endif