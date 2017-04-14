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
#include "mediastreamtrack.h"

using namespace crtc;

void MediaStreamTrackInternal::OnChanged() {
  switch (_source->state()) {
    case webrtc::MediaSourceInterface::kInitializing:
      break;
    case webrtc::MediaSourceInterface::kLive:
      switch (_state) {
        case webrtc::MediaSourceInterface::kInitializing:
          OnStarted();
          break;
        case webrtc::MediaSourceInterface::kLive:
          break;
        case webrtc::MediaSourceInterface::kEnded:
          break;
        case webrtc::MediaSourceInterface::kMuted:
          OnUnMute();
          break;
      }

      break;
    case webrtc::MediaSourceInterface::kEnded:
      OnEnded();
      break;
    case webrtc::MediaSourceInterface::kMuted:
      OnMute();
      break;
  }
}

webrtc::MediaStreamTrackInterface *MediaStreamTrackInternal::New(const Let<MediaStreamTrack> &track) {
  if (!track.IsEmpty()) {
    Let<MediaStreamTrackInternal> track_internal(track);
    return track_internal->_track.get();
  }

  return nullptr;
}

Let<MediaStreamTrack> MediaStreamTrackInternal::New(webrtc::MediaStreamTrackInterface *track) {
  if (track) {
    MediaStreamTrack::Type kind;
    webrtc::MediaSourceInterface *source = nullptr;

    if (track->kind().compare(webrtc::MediaStreamTrackInterface::kAudioKind) == 0) {
      webrtc::AudioTrackInterface *audio = static_cast<webrtc::AudioTrackInterface*>(track);
      source = audio->GetSource();
      kind = MediaStreamTrack::kAudio;
    } else {
      webrtc::VideoTrackInterface *video = static_cast<webrtc::VideoTrackInterface*>(track);
      source = video->GetSource();
      kind = MediaStreamTrack::kVideo;
    }

    return Let<MediaStreamTrackInternal>::New(kind, track, source);
  }

  return Let<MediaStreamTrack>();
}

MediaStreamTrackInternal::MediaStreamTrackInternal(MediaStreamTrack::Type kind, webrtc::MediaStreamTrackInterface *track, webrtc::MediaSourceInterface *source) : 
  _kind(kind),
  _track(track),
  _source(source),
  _state(_source->state())
{
  _source->RegisterObserver(this);
}

MediaStreamTrackInternal::MediaStreamTrackInternal(const Let<MediaStreamTrackInternal> &track) : 
  MediaStreamTrackInternal(track->_kind, track->_track, track->_source) 
{ }

MediaStreamTrackInternal::~MediaStreamTrackInternal() {
  _source->UnregisterObserver(this);
}

bool MediaStreamTrackInternal::Enabled() const {
  return _track->enabled();
}

bool MediaStreamTrackInternal::Remote() const {
  return _source->remote();
}

bool MediaStreamTrackInternal::Muted() const {
  return (_source->state() == webrtc::MediaSourceInterface::kMuted);
}

std::string MediaStreamTrackInternal::Id() const {
  return _track->id();
}

MediaStreamTrack::Type MediaStreamTrackInternal::Kind() const {
  return _kind;
}

MediaStreamTrack::State MediaStreamTrackInternal::ReadyState() const {
  if (_track->state() == webrtc::MediaStreamTrackInterface::kEnded || _source->state() == webrtc::MediaSourceInterface::kEnded) {
    return MediaStreamTrack::kEnded;
  }

  return MediaStreamTrack::kLive;
}

Let<MediaStreamTrack> MediaStreamTrackInternal::Clone() {
  return Let<MediaStreamTrackInternal>::New(_kind, _track, _source);
}

rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> MediaStreamTrackInternal::GetTrack() const {
  return _track;
}

rtc::scoped_refptr<webrtc::MediaSourceInterface> MediaStreamTrackInternal::GetSource() const {
  return _source;
}