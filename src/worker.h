
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

#ifndef CRTC_WORKER_H
#define CRTC_WORKER_H

#include "crtc.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/nullsocketserver.h"
#include "webrtc/base/criticalsection.h"
#include "webrtc/base/platform_thread.h"
#include "webrtc/typedefs.h"
#include "webrtc/system_wrappers/include/event_wrapper.h"

namespace crtc {
  class WorkerInternal : public Worker, public rtc::NullSocketServer, public rtc::Thread {
      friend class Worker;
      friend class Let<WorkerInternal>;

    private:
      static thread_local Let<WorkerInternal> current_worker;

    protected:
      explicit WorkerInternal();
      ~WorkerInternal() override;

      virtual bool Wait(int cms, bool process_io);
      void Run() override;
  };

  class RealTimeClockInternal : public RealTimeClock {
      friend class RealTimeClock;
      friend class Let<RealTimeClock>;

    public:
      void Start(uint32_t interval_ms = 0) override;
      void Stop() override;

    protected:
      explicit RealTimeClockInternal(const Callback &runnable);
      ~RealTimeClockInternal() override;

      std::unique_ptr<webrtc::EventTimerWrapper> _tick;
      rtc::PlatformThread _thread;
      rtc::PlatformThreadId _signal;

      Callback _runnable;
      
      static bool Run(void* obj);
  };
};

#endif