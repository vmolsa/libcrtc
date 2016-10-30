
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

#ifndef CRTC_ASYNC_H
#define CRTC_ASYNC_H

#include "crtc.h"

namespace crtc {
  class AsyncInternal {
    public:
      static void Init();
      static void Dispose();
  };

  class AsyncCall : public Event {
      friend class Let<AsyncCall>;

    public:
      static Let<AsyncCall> New(Functor<void()> callback);

      void Call() const;
    private: 
      explicit AsyncCall(Functor<void()> callback);
      ~AsyncCall() override;

    protected:
      Functor<void()> _callback;
  };
};

#endif