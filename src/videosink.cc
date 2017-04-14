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
#include "videosink.h"
#include "i420p.h"

using namespace crtc;

VideoSinkInternal::VideoSinkInternal(const Let<MediaStreamTrackInternal> &track, 
                                     const rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track) : 
  MediaStreamTrackInternal(track),
  _event(Let<Event>::New()),
  _video_track(video_track)
{
  video_track->AddOrUpdateSink(this, rtc::VideoSinkWants());
}

VideoSinkInternal::~VideoSinkInternal() {
  Stop();
}

Let<VideoSink> VideoSink::New(const Let<MediaStreamTrack> &mediaStreamTrack) {
  if (mediaStreamTrack.IsEmpty() || 
      mediaStreamTrack->Kind() != MediaStreamTrack::kVideo || 
      mediaStreamTrack->ReadyState() != MediaStreamTrack::kLive) 
  {
    return Let<VideoSink>();
  }

  Let<MediaStreamTrackInternal> track(mediaStreamTrack->Clone()); 
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> stream_track = track->GetTrack();
  rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track = static_cast<webrtc::VideoTrackInterface*>(stream_track.get());
  Let<VideoSinkInternal> self(Let<VideoSinkInternal>::New(track, video_track));
  
  if (!video_track->enabled()) {
    video_track->set_enabled(true);
  }

  return self;
}

void VideoSinkInternal::OnFrame(const webrtc::VideoFrame& frame) {
  ondata(WrapVideoFrameBuffer::New(frame.video_frame_buffer()));
}

bool VideoSinkInternal::Enabled() const { 
  return MediaStreamTrackInternal::Enabled();
}

bool VideoSinkInternal::Muted() const { 
  return MediaStreamTrackInternal::Muted();
}

bool VideoSinkInternal::Remote() const { 
  return MediaStreamTrackInternal::Remote();
}

std::string VideoSinkInternal::Id() const { 
  return MediaStreamTrackInternal::Id();
}

MediaStreamTrack::Type VideoSinkInternal::Kind() const { 
  return MediaStreamTrackInternal::Kind();
}

MediaStreamTrack::State VideoSinkInternal::ReadyState() const { 
  return MediaStreamTrackInternal::ReadyState();
}

Let<MediaStreamTrack> VideoSinkInternal::Clone() { 
  return MediaStreamTrackInternal::Clone();
}

bool VideoSinkInternal::IsRunning() const {
  return (!_event.IsEmpty());
}

void VideoSinkInternal::Stop() {
  if (!_event.IsEmpty()) {
    _video_track->RemoveSink(this);
    _event.Dispose();
  }

  ondata.Dispose();
}

void VideoSinkInternal::OnEnded() {
  Stop();
}