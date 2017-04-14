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

#ifndef CRTC_AUDIODEVICE_H
#define CRTC_AUDIODEVICE_H

#include "crtc.h"
#include "worker.h"

#include "webrtc/base/criticalsection.h"
#include "webrtc/modules/audio_device/include/fake_audio_device.h"
#include "webrtc/typedefs.h"

namespace crtc {
  class AudioDevice : public FakeAudioDeviceModule {
    public:
      AudioDevice(float khz = 44.1, int channels = 2) :
        _freq(44.1 * 1000),
        _channels(channels),
        _tick(EventTimerWrapper::Create()),
        _thread()
      {
        
      }

      ~AudioDevice() override {

      }
    private:
      inline int32_t Init() override {

      }

      inline int32_t RegisterAudioCallback(AudioTransport* callback) override {

      }

      inline int32_t StartPlayout() override {

      }

      inline int32_t StopPlayout() override {

      }

      inline int32_t StartRecording() override {

      }

      inline int32_t StopRecording() override {

      }

      inline bool Playing() const override {

      }

      inline bool Recording() const override {

      }

      inline static bool Run(void* obj) {
        static_cast<FakeAudioDevice*>(obj)->ProcessAudio();
        return true;
      }

      inline void ProcessAudio() {
        {
          rtc::CritScope cs(&lock_);
          if (capturing_) {
            // Capture 10ms of audio. 2 bytes per sample.
            rtc::ArrayView<const int16_t> audio_data = capturer_->Capture();
            uint32_t new_mic_level = 0;
            audio_callback_->RecordedDataIsAvailable(
                audio_data.data(), audio_data.size(), 2, 1, sampling_frequency_in_hz_,
                0, 0, 0, false, new_mic_level);
          }
          if (rendering_) {
            size_t samples_out = 0;
            int64_t elapsed_time_ms = -1;
            int64_t ntp_time_ms = -1;
            audio_callback_->NeedMorePlayData(
                num_samples_per_frame_, 2, 1, sampling_frequency_in_hz_,
                playout_buffer_.data(), samples_out, &elapsed_time_ms, &ntp_time_ms);
          }
        }

        _tick->Wait(WEBRTC_EVENT_INFINITE);
      }

  protected:
      rtc::CriticalSection _lock;

      const int _freq;
      const int _channels;
      const size_t _samples;
      
      webrtc::AudioTransport* _callback GUARDED_BY(_lock);

      std::unique_ptr<EventTimerWrapper> _tick;
      Let<Worker> _worker;
      rtc::Thread* _thread;
  };
};

class FakeAudioDevice::PulsedNoiseCapturer {
 public:
  PulsedNoiseCapturer(size_t num_samples_per_frame, int16_t max_amplitude)
      : fill_with_zero_(false),
        random_generator_(1),
        max_amplitude_(max_amplitude),
        random_audio_(num_samples_per_frame),
        silent_audio_(num_samples_per_frame, 0) {
    RTC_DCHECK_GT(max_amplitude, 0);
  }
  rtc::ArrayView<const int16_t> Capture() {
    fill_with_zero_ = !fill_with_zero_;
    if (!fill_with_zero_) {
      std::generate(random_audio_.begin(), random_audio_.end(), [&]() {
        return random_generator_.Rand(-max_amplitude_, max_amplitude_);
      });
    }
    return fill_with_zero_ ? silent_audio_ : random_audio_;
  }
 private:
  bool fill_with_zero_;
  Random random_generator_;
  const int16_t max_amplitude_;
  std::vector<int16_t> random_audio_;
  std::vector<int16_t> silent_audio_;
};
FakeAudioDevice::FakeAudioDevice(float speed,
                                 int sampling_frequency_in_hz,
                                 int16_t max_amplitude)
    : sampling_frequency_in_hz_(sampling_frequency_in_hz),
      num_samples_per_frame_(
          rtc::CheckedDivExact(sampling_frequency_in_hz_, kFramesPerSecond)),
      speed_(speed),
      audio_callback_(nullptr),
      rendering_(false),
      capturing_(false),
      capturer_(new FakeAudioDevice::PulsedNoiseCapturer(num_samples_per_frame_,
                                                         max_amplitude)),
      playout_buffer_(num_samples_per_frame_, 0),
      tick_(EventTimerWrapper::Create()),
      thread_(FakeAudioDevice::Run, this, "FakeAudioDevice") {
  RTC_DCHECK(
      sampling_frequency_in_hz == 8000 || sampling_frequency_in_hz == 16000 ||
      sampling_frequency_in_hz == 32000 || sampling_frequency_in_hz == 44100 ||
      sampling_frequency_in_hz == 48000);
}
FakeAudioDevice::~FakeAudioDevice() {
  StopPlayout();
  StopRecording();
  thread_.Stop();
}
int32_t FakeAudioDevice::StartPlayout() {
  rtc::CritScope cs(&lock_);
  rendering_ = true;
  return 0;
}
int32_t FakeAudioDevice::StopPlayout() {
  rtc::CritScope cs(&lock_);
  rendering_ = false;
  return 0;
}
int32_t FakeAudioDevice::StartRecording() {
  rtc::CritScope cs(&lock_);
  capturing_ = true;
  return 0;
}
int32_t FakeAudioDevice::StopRecording() {
  rtc::CritScope cs(&lock_);
  capturing_ = false;
  return 0;
}
int32_t FakeAudioDevice::Init() {
  RTC_CHECK(tick_->StartTimer(true, kFrameLengthMs / speed_));
  thread_.Start();
  thread_.SetPriority(rtc::kHighPriority);
  return 0;
}
int32_t FakeAudioDevice::RegisterAudioCallback(AudioTransport* callback) {
  rtc::CritScope cs(&lock_);
  RTC_DCHECK(callback || audio_callback_ != nullptr);
  audio_callback_ = callback;
  return 0;
}
bool FakeAudioDevice::Playing() const {
  rtc::CritScope cs(&lock_);
  return rendering_;
}
bool FakeAudioDevice::Recording() const {
  rtc::CritScope cs(&lock_);
  return capturing_;
}
bool FakeAudioDevice::Run(void* obj) {
  static_cast<FakeAudioDevice*>(obj)->ProcessAudio();
  return true;
}
void FakeAudioDevice::ProcessAudio() {
  
}

#endif