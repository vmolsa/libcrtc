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
#include "videosource.h"
#include "rtcpeerconnection.h"
#include "webrtc/base/stringencode.h"
#include "webrtc/api/test/fakeconstraints.h"

using namespace crtc;

volatile int VideoSourceInternal::counter;

VideoSourceInternal::VideoSourceInternal(webrtc::MediaStreamInterface *stream) :
  MediaStreamInternal(stream),
  _capturer(new VideoCapturer())
{
  _capturer->SignalStateChange.connect(this, &VideoSourceInternal::OnStateChange);
  _capturer->Drain.connect(this, &VideoSourceInternal::OnDrain);
}

VideoSourceInternal::~VideoSourceInternal() {
  if (_capturer) {
    OnStateChange(_capturer, cricket::CS_STOPPED);
  }
}

VideoCapturer* VideoSourceInternal::GetCapturer() const {
  return _capturer;
}

Let<VideoSource> VideoSource::New(int width, int height, float fps) {
  std::string stream_label = "videosource" + rtc::ToString<int>(rtc::AtomicOps::AcquireLoad(&VideoSourceInternal::counter));
  std::string track_label = stream_label + "_videotrack";
  rtc::AtomicOps::Increment(&VideoSourceInternal::counter);

  rtc::scoped_refptr<webrtc::MediaStreamInterface> stream(RTCPeerConnectionInternal::factory->CreateLocalMediaStream(stream_label));

  if (stream.get()) {
    Let<VideoSourceInternal> self = Let<VideoSourceInternal>::New(stream.get());
    webrtc::FakeConstraints constraints;

    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMaxWidth, width);
    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMaxHeight, height);
    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMaxFrameRate, fps);
    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMinWidth, width);
    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMinHeight, height);
    constraints.AddMandatory(webrtc::MediaConstraintsInterface::kMinFrameRate, fps);

    rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source(RTCPeerConnectionInternal::factory->CreateVideoSource(self->GetCapturer(), &constraints));

    if (source.get()) {
      rtc::scoped_refptr<webrtc::VideoTrackInterface> track(RTCPeerConnectionInternal::factory->CreateVideoTrack(track_label, source.get()));

      if (stream->AddTrack(track)) {
        return self;
      } else {
        self->GetCapturer()->Stop();
      }
    }
  }

  return Let<VideoSource>();
}

std::string VideoSourceInternal::Id() const { 
  return MediaStreamInternal::Id();
}

void VideoSourceInternal::AddTrack(const Let<MediaStreamTrack> &track) {
  return MediaStreamInternal::AddTrack(track);
}

void VideoSourceInternal::RemoveTrack(const Let<MediaStreamTrack> &track) {
  return MediaStreamInternal::RemoveTrack(track);
}

Let<MediaStreamTrack> VideoSourceInternal::GetTrackById(const std::string &id) const {
  return MediaStreamInternal::GetTrackById(id);
}

MediaStreamTracks VideoSourceInternal::GetAudioTracks() const { 
  return MediaStreamInternal::GetAudioTracks();
}

MediaStreamTracks VideoSourceInternal::GetVideoTracks() const {
  return MediaStreamInternal::GetVideoTracks();
}

Let<MediaStream> VideoSourceInternal::Clone() {
  return MediaStreamInternal::Clone();
}

bool VideoSourceInternal::IsRunning() const {
  if (_capturer) {
    return _capturer->IsRunning();
  }

  return false;
}

void VideoSourceInternal::Stop() {
  if (_capturer) {
    _capturer->Stop();
  }
}

int VideoSourceInternal::Width() const {
  if (_capturer) {
    return _capturer->Width();
  }

  return 0;
}

int VideoSourceInternal::Height() const {
  if (_capturer) {
    return _capturer->Height();
  }

  return 0;
}

float VideoSourceInternal::Fps() const {
  if (_capturer) {
    return _capturer->Fps();
  }

  return 0;
}

void VideoSourceInternal::Write(const Let<ImageBuffer> &i420p_frame, ErrorCallback callback) {
  if (_capturer) {
    _capturer->Write(i420p_frame, callback);
  } else {
    callback(Error::New("VideoSource ended.", __FILE__, __LINE__));
  }
}

void VideoSourceInternal::OnStateChange(cricket::VideoCapturer* capturer, cricket::CaptureState capture_state) {
  switch (capture_state) {
    case cricket::CS_FAILED:
    case cricket::CS_STOPPED:
      _capturer->SignalStateChange.disconnect(this);
      _capturer->Drain.disconnect(this);
      _capturer = nullptr;
      _event.Dispose();

      break;
    case cricket::CS_STARTING:
      _event = Event::New();
      break;
    case cricket::CS_RUNNING:
      break;
  }
}

void VideoSourceInternal::OnDrain() {
  ondrain();
}