#include <stdio.h>
#include <string>
#include <utility>

#include "crtc.h"

using namespace crtc;

static int total = 0, requested = 100;

int main() {
  Module::Init();
  
  printf("Creating Workers...\n");

  for (int index = 0; index < requested; index++) {
    Let<Worker> worker =  Worker::New([index]() {
      SetTimeout([=]() {
        if (!Worker::This().IsEmpty()) {
          printf("I'M WORKER[%d]!\n", index);
          total++;
        } else {
          printf("I'M NOT WORKER[%d]!\n", index);
        }
      }, 30000);
    });
  }

  printf("All Workers are now created. Idling for 30 seconds...\n");

  Module::DispatchEvents(true);
  Module::Dispose();

  if (total != requested) {
    printf("Test Failed: current count: %d, requested count: %d\n", total, requested);
  } else {
    printf("All %d workers created!\n", total);
  }

  return 0;
};