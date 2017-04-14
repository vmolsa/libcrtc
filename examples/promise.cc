#include <stdio.h>
#include <string>
#include <utility>

#include "crtc.h"

using namespace crtc;

int main() {
  Module::Init();

  Promise<std::string>::New([](const Promise<std::string>::FullFilledCallback &resolve, const Promise<std::string>::RejectedCallback &reject) {
    SetTimeout([](const Promise<std::string>::FullFilledCallback &resolve) {
      resolve(std::string("Hello Test!"));
    }, 200, resolve);
  })->Then([](std::string result) {
    printf("Promise<std::string>(%s)\n", result.c_str());
  })->Catch([](const Let<Error> &error) {
    printf("Promise<std::string>(%s)\n", error->ToString().c_str());
  });

  Promise<void>::New([](const Promise<void>::FullFilledCallback &resolve, const Promise<void>::RejectedCallback &reject) {
    resolve();
  })->Then([]() {
    printf("Promise<void>()\n");
  })->Catch([](const Let<Error> &error) {
    printf("Promise<void>(%s)\n", error->ToString().c_str());
  });

  Promise<void>::New([](const Promise<void>::FullFilledCallback &resolve, const Promise<void>::RejectedCallback &reject) {
    reject(Error::New("Testing Error", __FILE__, __LINE__));
  })->Then([]() {
    printf("Promise<void(error)>()\n");
  })->Catch([](const Let<Error> &error) {
    printf("Promise<void(error)>(%s)\n", error->ToString().c_str());
  });

  Promise<int>::New([](const Promise<int>::FullFilledCallback &resolve, const Promise<int>::RejectedCallback &reject) {
    SetImmediate([](const Promise<int>::FullFilledCallback &resolve) {
      resolve(1337);
    }, resolve);
  })->Then([](int value) {
    printf("Promise<int>(%d)\n", value);
  })->Catch([](const Let<Error> &error) {
    printf("Promise<int>(%s)\n", error->ToString().c_str());
  });

  Module::DispatchEvents(true);
  Module::Dispose();

  return 0;
}