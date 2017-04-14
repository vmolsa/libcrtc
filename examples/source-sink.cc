#include <stdio.h>
#include <string>
#include <utility>

#include "crtc.h"

using namespace crtc;

void ReportError(const Let<Error> &error) {
  if (!error.IsEmpty()) {
    fprintf(stderr, "%s\n", error->ToString().c_str());
  }
}

int main() {
  Module::Init();

  Let<VideoSource> source = VideoSource::New();

  {
    printf("MediaStream: %s, width: %d, height: %d, fps: %.2f\n", source->Id().c_str(), source->Width(), source->Height(), source->Fps());

    for (const auto &track: source->GetVideoTracks()) {
      printf("MediaStreamTrack: %s\n", track->Id().c_str());

      Let<MediaStreamTrack> mtrack = track->Clone();
      mtrack->onended = [mtrack]() {
        printf("MediaStreamTrack: %s Ended!\n", mtrack->Id().c_str());
      };

      Let<VideoSink> sink = VideoSink::New(track);

      int64_t begin = Time::Now();

      sink->ondata = [sink, begin](const Let<I420P> &frame) {
        Let<ArrayBuffer> buffer(frame);

        printf("ArrayBuffer: %lu bytes, image width: %d, image height: %d, +%.2lf seconds\n", buffer->ByteLength(), frame->Width(), frame->Height(), Time::Since(begin));
      };
    }
  }

  Worker::New([=]() {
    Let<I420P> frame = I420P::New(1280, 720);

    for (int index = 0; index < (30 * 30); index++) {
      source->Write(frame, &ReportError);
    }
  });

  source->ondrain = [=]() {
    source->Stop();
  };

  Module::DispatchEvents(true);
  Module::Dispose();

  return 0;
}