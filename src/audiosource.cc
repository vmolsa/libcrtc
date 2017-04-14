
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

#include "crtc.h"
#include "audiosource.h"

using namespace crtc;

AudioSourceInternal::AudioSourceInternal() {

}

AudioSourceInternal::~AudioSourceInternal() {
  
}

bool AudioSourceInternal::IsRunning() const {
  return false;
}

void AudioSourceInternal::Stop() {

}

void AudioSourceInternal::Write(const Let<AudioBuffer> &buffer, ErrorCallback callback) {

}

std::string AudioSourceInternal::Id() const { 
  return MediaStreamInternal::Id();
}

void AudioSourceInternal::AddTrack(const Let<MediaStreamTrack> &track) {
  return MediaStreamInternal::AddTrack(track);
}

void AudioSourceInternal::RemoveTrack(const Let<MediaStreamTrack> &track) {
  return MediaStreamInternal::RemoveTrack(track);
}

Let<MediaStreamTrack> AudioSourceInternal::GetTrackById(const std::string &id) const {
  return MediaStreamInternal::GetTrackById(id);
}

MediaStreamTracks AudioSourceInternal::GetAudioTracks() const { 
  return MediaStreamInternal::GetAudioTracks();
}

MediaStreamTracks AudioSourceInternal::GetVideoTracks() const {
  return MediaStreamInternal::GetVideoTracks();
}

Let<MediaStream> AudioSourceInternal::Clone() {
  return MediaStreamInternal::Clone();
}

Let<AudioSource> AudioSource::New() {
  return Let<AudioSourceInternal>::New();
}
