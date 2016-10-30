/*
* The MIT License (MIT)
*
* Copyright (c) 2016 vmolsa <ville.molsa@gmail.com> (http://github.com/vmolsa)
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
#include "async.h"

#include "webrtc/base/thread.h"
#include "webrtc/base/asyncinvoker.h"

using namespace crtc;

std::unique_ptr<rtc::Thread> _worker;
rtc::AsyncInvoker* _async;

void AsyncInternal::Init() {
  _async = new rtc::AsyncInvoker();
}

void AsyncInternal::Dispose() {
  delete _async;
}

void Async::Call(Functor<void()> callback) {
  Let<AsyncCall> ac = AsyncCall::New(callback);

  _async->AsyncInvoke<void>(RTC_FROM_HERE, rtc::Thread::Current(), [ac]() {
    ac->Call();
  });
}

Let<AsyncCall> AsyncCall::New(Functor<void()> callback) {
  return Let<AsyncCall>::New(callback);
}

AsyncCall::AsyncCall(Functor<void()> callback) : _callback(callback) {
  
}

AsyncCall::~AsyncCall() {
  
}

void AsyncCall::Call() const {
  _callback();
}