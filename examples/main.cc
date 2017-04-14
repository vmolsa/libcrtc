#include <stdio.h>
#include <string>
#include <utility>

#include <crtc.h>

using namespace crtc;

// cc -std=c++11 -stdlib=libc++ -Ldist/lib/ -Wall -lcrtc -Idist/include/ -Wl,-rpath,dist/lib/ -fvisibility=hidden -fvisibility-inlines-hidden -fno-rtti -fno-exceptions -fno-threadsafe-statics -o out/main-example examples/main.cc

int main() {
  printf("Loading Library...\n");

  Module::Init();

  printf("Creating Async Event...\n");

  Async::Call([]() {
    printf("Hello from Worker...\n");
  }, 5000, Worker::New());

  printf("Dispatch Events...\n");

  Module::DispatchEvents(true);

  printf("Disposing Module...\n");

  Module::Dispose();

  return 0;
}