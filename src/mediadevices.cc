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
#include "mediadevices.h"
#include <string> 

using namespace crtc;

std::unique_ptr<rtc::Thread> MediaDevicesInternal::network_thread;
std::unique_ptr<rtc::Thread> MediaDevicesInternal::worker_thread;
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> MediaDevicesInternal::media_factory;
rtc::scoped_refptr<webrtc::AudioDeviceModule> MediaDevicesInternal::audio_device;
std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> MediaDevicesInternal::video_device;

void MediaDevicesInternal::Init() {
  network_thread = rtc::Thread::CreateWithSocketServer();
  network_thread->SetName("network", nullptr);
  
  if (!network_thread->Start()) {
    
  }

  worker_thread = rtc::Thread::Create();
  worker_thread->SetName("worker", nullptr);
  
  if (!worker_thread->Start()) {
    
  }

  audio_device = webrtc::AudioDeviceModule::Create(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);

  if (!audio_device.get()) {
    // TODO(): Handle Error!
  }

  if (!audio_device->Initialized()) {
    audio_device->Init();
  }

  if (!audio_device->PlayoutIsInitialized()) {
    audio_device->InitPlayout();
  }

  if (!audio_device->RecordingIsInitialized()) {
    audio_device->InitRecording();
  }

  video_device.reset(webrtc::VideoCaptureFactory::CreateDeviceInfo(0));

  if (!video_device) {
    // TODO(): Handle Error!
  }

  media_factory = webrtc::CreatePeerConnectionFactory(network_thread.get(), 
                                                      worker_thread.get(), 
                                                      rtc::Thread::Current(),
                                                      audio_device.get(),
                                                      nullptr, // cricket::WebRtcVideoEncoderFactory*
                                                      nullptr); // cricket::WebRtcVideoDecoderFactory*

  media_factory = webrtc::CreatePeerConnectionFactory();

  if (!media_factory.get()) {
    // TODO(): Handle Error!
  }
}

Let<Promise<MediaDeviceInfos>> MediaDevices::EnumerateDevices() {
  return Promise<MediaDeviceInfos>::New([](Deferred<MediaDeviceInfos> *Q) {
    MediaDeviceInfos devices;
    
    for (int index = 0, devs = MediaDevicesInternal::audio_device->PlayoutDevices(); index < devs; index++) {
      char label[webrtc::kAdmMaxDeviceNameSize] = {0};
      char guid[webrtc::kAdmMaxGuidSize] = {0};

      if (!MediaDevicesInternal::audio_device->PlayoutDeviceName(index, label, guid)) {
        MediaDeviceInfo dev;

        dev.deviceId = std::to_string(index);
        dev.label = std::string(label);
        dev.groupId = std::string(guid);
        dev.kind = MediaDeviceInfo::kAudioOutput;

        devices.push_back(dev);
      }
    }

    for (int index = 0, devs = MediaDevicesInternal::audio_device->RecordingDevices(); index < devs; index++) {
      char label[webrtc::kAdmMaxDeviceNameSize] = {0};
      char guid[webrtc::kAdmMaxGuidSize] = {0};

      if (!MediaDevicesInternal::audio_device->RecordingDeviceName(index, label, guid)) {
        MediaDeviceInfo dev;

        dev.deviceId = std::to_string(index);
        dev.label = std::string(label);
        dev.groupId = std::string(guid);
        dev.kind = MediaDeviceInfo::kAudioInput;

        devices.push_back(dev);
      }
    }
    
    for (int index = 0, devs = MediaDevicesInternal::video_device->NumberOfDevices(); index < devs; index++) {
      char label[webrtc::kAdmMaxDeviceNameSize] = {0};
      char guid[webrtc::kAdmMaxDeviceNameSize] = {0};

      if (!MediaDevicesInternal::video_device->GetDeviceName(index, label, webrtc::kAdmMaxDeviceNameSize, guid, webrtc::kAdmMaxDeviceNameSize)) {
        MediaDeviceInfo dev;

        dev.deviceId = std::to_string(index);
        dev.label = std::string(label);
        dev.groupId = std::string(guid);
        dev.kind = MediaDeviceInfo::kVideoInput;

        devices.push_back(dev);
      }
    }

    Q->Resolve(devices);
  });
}

Let<Promise<Let<MediaStream>>> MediaDevices::GetUserMedia(const MediaStreamConstraints &constraints) {
  return Promise<Let<MediaStream>>::New([constraints](Deferred<Let<MediaStream>> *Q) {
    if (!constraints.enableAudio && constraints.enableVideo) {
      return Q->Resolve(Let<MediaStream>());
    }

    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
    
    if (constraints.enableAudio) {
      cricket::AudioOptions options;
      rtc::scoped_refptr<webrtc::AudioSourceInterface> audio_source = MediaDevicesInternal::media_factory->CreateAudioSource(options);

      if (audio_source.get()) {
        audio_track = MediaDevicesInternal::media_factory->CreateAudioTrack("audio", audio_source);
      }
    }

    if (constraints.enableVideo) {
      cricket::WebRtcVideoDeviceCapturerFactory factory;
      cricket::VideoCapturer *capturer = nullptr;

      if (constraints.videoSourceId.empty()) {
        int dev_count = MediaDevicesInternal::video_device->NumberOfDevices();
        std::vector<std::string> devs;

        for (int index = 0; index < dev_count; index++) {
          const uint32_t kSize = 256;
          char name[kSize] = {0};
          char id[kSize] = {0};
          
          if (MediaDevicesInternal::video_device->GetDeviceName(index, name, kSize, id, kSize) != -1) {
            devs.push_back(name);
          }
        }

        for (const auto& name : devs) {
          capturer = factory.Create(cricket::Device(name, 0));
          
          if (capturer) {
            break;
          }
        }
      } else {
        capturer = factory.Create(cricket::Device(constraints.videoSourceId, 0));
      }

      if (capturer) {
        rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> video_source(MediaDevicesInternal::media_factory->CreateVideoSource(capturer));

        if (video_source.get()) {
          video_track = MediaDevicesInternal::media_factory->CreateVideoTrack("video", video_source);
        }
      }
    }

    if (video_track.get() || audio_track.get()) {
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = MediaDevicesInternal::media_factory->CreateLocalMediaStream("stream");

      if (stream.get()) {
        if (video_track.get()) {
          stream->AddTrack(video_track);
        }

        if (audio_track.get()) {
          stream->AddTrack(audio_track);
        }

        return Q->Resolve(MediaStreamInternal::New(stream));
      }
    }

    Q->Resolve(Let<MediaStream>()); // TODO(): Reject!
  });
}

Let<Promise<Let<MediaStream>>> MediaDevices::GetUserMedia() {
  MediaStreamConstraints constraints;

  constraints.enableAudio = true;
  constraints.enableVideo = true;

  return MediaDevices::GetUserMedia(constraints);
}