#include <stdio.h>
#include <string>

#include "crtc.h"

using namespace crtc;

int main() {
  Events::Init();

  auto a = Promise<std::string>::New([](Deferred<std::string> *Q) {
    Q->Resolve(std::string("Hello Test!"));
  });

  a->Then([](std::string result) {
    //printf("Done: %d\n", result);
    printf("Done: %s\n", result.c_str());
    //printf("Result: %s\n", result ? "true" : "false");
  })->Catch([](const std::exception &error) {

  })->Finally([]() {
    printf("Done!\n");
  });

  auto b = Promise<void>::New([](Deferred<void> *Q) {
    Q->Resolve();
  });

  b->Then([]() {
    printf("Void Done!\n");
  });

  Events::DispatchEvents(true);
  Events::Dispose();

  return 0;
}