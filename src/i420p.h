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

#ifndef CRTC_I420P_H
#define CRTC_I420P_H

#include "crtc.h"
#include "arraybuffer.h"

#include "webrtc/base/refcount.h"
#include "webrtc/common_video/include/video_frame_buffer.h"

namespace crtc {
  class I420PInternal : public I420P, public ArrayBufferInternal {
      friend class Let<I420PInternal>;

    public:
      static Let<I420P> New(const Let<ArrayBuffer> &buffer, int width, int height);
      static Let<I420P> New(int width = 0, int height = 0);

      int Width() const override;
      int Height() const override;

      const uint8_t* DataY() const override;
      const uint8_t* DataU() const override;
      const uint8_t* DataV() const override;

      int StrideY() const override;
      int StrideU() const override;
      int StrideV() const override;

      size_t ByteLength() const override;

      Let<ArrayBuffer> Slice(size_t begin = 0, size_t end = 0) const override;

      uint8_t *Data() override;
      const uint8_t *Data() const override;

      std::string ToString() const override;

    protected:
      explicit I420PInternal(const Let<ArrayBuffer> &buffer, int width, int height);
      I420PInternal(int width = 0, int height = 0);
      ~I420PInternal() override;

      int _width;
      int _height;
      
      const uint8_t* _y;
      const uint8_t* _u;
      const uint8_t* _v;
  };

  class WrapI420P : public webrtc::VideoFrameBuffer {
    public:
      static rtc::scoped_refptr<webrtc::VideoFrameBuffer> New(const Let<I420P> &source);

      int width() const override;
      int height() const override;

      const uint8_t* DataY() const override;
      const uint8_t* DataU() const override;
      const uint8_t* DataV() const override;

      int StrideY() const override;
      int StrideU() const override;
      int StrideV() const override;

      void* native_handle() const override;
      rtc::scoped_refptr<webrtc::VideoFrameBuffer> NativeToI420Buffer() override;

    protected:
      explicit WrapI420P(const Let<I420P> &source);
      ~WrapI420P() override;

      Let<I420P> _source;
  };

  class WrapVideoFrameBuffer : public I420P {
      friend class Let<WrapVideoFrameBuffer>;

    public:
      static Let<I420P> New(const rtc::scoped_refptr<webrtc::VideoFrameBuffer> &vfb);

      int Width() const override;
      int Height() const override;

      const uint8_t* DataY() const override;
      const uint8_t* DataU() const override;
      const uint8_t* DataV() const override;

      int StrideY() const override;
      int StrideU() const override;
      int StrideV() const override;

      size_t ByteLength() const override;

      Let<ArrayBuffer> Slice(size_t begin = 0, size_t end = 0) const override;

      uint8_t *Data() override;
      const uint8_t *Data() const override;

      std::string ToString() const override;
    protected:
      explicit WrapVideoFrameBuffer(const rtc::scoped_refptr<webrtc::VideoFrameBuffer> &source);
      ~WrapVideoFrameBuffer() override;

      rtc::scoped_refptr<webrtc::VideoFrameBuffer> _vfb;
  };

  class WrapBufferToVideoFrameBuffer : public webrtc::VideoFrameBuffer {
    public:
      static rtc::scoped_refptr<webrtc::VideoFrameBuffer> New(const Let<ArrayBuffer> &source, int width, int height);

      int width() const override;
      int height() const override;

      const uint8_t* DataY() const override;
      const uint8_t* DataU() const override;
      const uint8_t* DataV() const override;

      int StrideY() const override;
      int StrideU() const override;
      int StrideV() const override;

      void* native_handle() const override;
      rtc::scoped_refptr<webrtc::VideoFrameBuffer> NativeToI420Buffer() override;

    protected:
      explicit WrapBufferToVideoFrameBuffer(const Let<ArrayBuffer> &source, int width, int height);
      ~WrapBufferToVideoFrameBuffer() override;

      Let<ArrayBuffer> _source;
      
      int _width;
      int _height;

      const uint8_t* _y;
      const uint8_t* _u;
      const uint8_t* _v;
  };
};

#endif