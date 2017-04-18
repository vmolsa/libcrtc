#include <stdio.h>
#include <string>
#include <utility>

#include "crtc.h"

using namespace crtc;

const size_t byteLength = 1382400;

// ffmpeg -f avfoundation -pix_fmt nv12 -s 1280x720 -i "0x7ffb5e40e8a0" -c:v rawvideo -f rawvideo -pix_fmt yuv420p -s 1280x720 - | ./out/ffmpeg | ffmpeg -f rawvideo -pix_fmt yuv420p -s:v 1280x720 -i pipe:0 -c:v libx264 -y output.mp4

void ReadFrame(const Let<VideoSource> &source, const Let<ArrayBuffer> &buffer) {
  if (source->IsRunning()) {
    size_t bytes = fread(buffer->Data(), 1, buffer->ByteLength(), stdin);

    if (bytes == buffer->ByteLength()) {
      source->Write(ImageBuffer::New(buffer, 1280, 720), [=](const Let<Error> &error) {
        if (!error.IsEmpty()) {
          fprintf(stderr, "VideoSource::Write(%s)\n", error->ToString().c_str());
        } else {
          SetTimeout(&ReadFrame, 33, source, buffer);
        }
      });
    } else {
      fprintf(stderr, "Invalid Bytes: %lu, Expected: %lu\n", bytes, buffer->ByteLength());
      source->Stop();
    }
  }
}

int main() {
  Module::Init();

  Let<VideoSource> source = VideoSource::New();

  fprintf(stderr, "MediaStream: %s\n", source->Id().c_str());

  for (const auto &track: source->GetVideoTracks()) {
    fprintf(stderr, "MediaStreamTrack: %s\n", track->Id().c_str());

    Let<VideoSink> sink = VideoSink::New(track);
    Let<MediaStreamTrack> mtrack = track->Clone();

    sink->ondata = [=](const Let<ImageBuffer> &frame) {
      Let<ArrayBuffer> buffer(frame);
      fwrite(frame->Data(), 1, buffer->ByteLength(), stdout);
    };

    mtrack->onended = [=]() {
      fprintf(stderr, "MediaStreamTrack: %s Ended!\n", mtrack->Id().c_str());
      sink->ondata.Dispose();
      mtrack->onended.Dispose();
    };
  }
  
  Worker::New([=]() {
    Let<ArrayBuffer> buffer = ArrayBuffer::New(byteLength);
    ReadFrame(source, buffer);
  });

  Module::DispatchEvents(true);
  Module::Dispose();

  return 0;
}