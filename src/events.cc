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
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/event_tracer.h"
#include "webrtc/system_wrappers/include/trace.h"

#ifdef CRTC_OS_WIN
  #include "webrtc/base/win32socketinit.h"
  #include "webrtc/base/win32socketserver.h"
#endif

using namespace crtc;

volatile int _pending = 0;

void Events::Init() {
#ifdef CRTC_OS_WIN
  rtc::EnsureWinsockInit();
#endif

  rtc::ThreadManager::Instance()->WrapCurrentThread();
  webrtc::Trace::CreateTrace();

  rtc::InitializeSSL();

  AsyncInternal::Init();
}

void Events::Dispose() {
  AsyncInternal::Dispose();
  rtc::CleanupSSL();
}

bool Events::DispatchEvents(bool forever) {
  bool result = false;
   
  do {
    result = (rtc::AtomicOps::AcquireLoad(&_pending) > 0 && rtc::Thread::Current()->ProcessMessages(0));
  } while (forever && result);

  return result;
}

Event::Event() {
  rtc::AtomicOps::Increment(&_pending);
}

Event::~Event() {
  rtc::AtomicOps::Decrement(&_pending);
}