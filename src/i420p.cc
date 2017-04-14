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

#include "crtc.h"
#include "i420p.h"

using namespace crtc;

I420PInternal::I420PInternal(const Let<ArrayBuffer> &buffer, int width, int height) : 
  ArrayBufferInternal(buffer),
  _width(width), 
  _height(height)
{
  _y = ArrayBufferInternal::Data();
  _u = ArrayBufferInternal::Data() + _width * _height;
  _v = ArrayBufferInternal::Data() + _width * _height + ((_width + 1) >> 1) * ((_height + 1) >> 1);
}

I420PInternal::I420PInternal(int width, int height) :
  ArrayBufferInternal(nullptr, I420P::ByteLength(width, height)),
  _width(width), 
  _height(height) 
{
  _y = ArrayBufferInternal::Data();
  _u = ArrayBufferInternal::Data() + _width * _height;
  _v = ArrayBufferInternal::Data() + _width * _height + ((_width + 1) >> 1) * ((_height + 1) >> 1);
}

I420PInternal::~I420PInternal() {
  
}

Let<I420P> I420PInternal::New(const Let<ArrayBuffer> &buffer, int width, int height) {
  return Let<I420PInternal>::New(buffer, width, height);
}

Let<I420P> I420PInternal::New(int width, int height) {
  return Let<I420PInternal>::New(width, height);
}

int I420PInternal::Width() const {
  return _width;
}

int I420PInternal::Height() const {
  return _height;
}

const uint8_t* I420PInternal::DataY() const {
  return _y;
}

const uint8_t* I420PInternal::DataU() const {
  return _u;
}

const uint8_t* I420PInternal::DataV() const {
  return _v;
}

int I420PInternal::StrideY() const {
  return _width;
}

int I420PInternal::StrideU() const {
  return (_width + 1) >> 1;
}

int I420PInternal::StrideV() const {
  return (_width + 1) >> 1;
}

size_t I420PInternal::ByteLength() const {
  return ArrayBufferInternal::ByteLength();
}

Let<ArrayBuffer> I420PInternal::Slice(size_t begin, size_t end) const {
  return ArrayBufferInternal::Slice(begin, end);
}

uint8_t *I420PInternal::Data() {
  return ArrayBufferInternal::Data();
}

const uint8_t *I420PInternal::Data() const {
  return ArrayBufferInternal::Data();
}

std::string I420PInternal::ToString() const {
  return ArrayBufferInternal::ToString();
}

Let<I420P> I420P::New(int width, int height) {
  return I420PInternal::New(width, height);
}

Let<I420P> I420P::New(const Let<ArrayBuffer> &buffer, int width, int height) {
  if (I420P::ByteLength(width, height) == buffer->ByteLength()) {
    return I420PInternal::New(buffer, width, height);
  }
  
  return Let<I420P>::Empty();
}

size_t I420P::ByteLength(int height, int stride_y, int stride_u, int stride_v) {
  return static_cast<size_t>(stride_y * height + (stride_u + stride_v) * ((height + 1) >> 1));
}

size_t I420P::ByteLength(int width, int height) {
  if (width > 0 && height > 0) {
    return ByteLength(height, width, (width + 1) >> 1, (width + 1) >> 1);
  }

  return 0; 
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> WrapI420P::New(const Let<I420P> &source) {
  if (!source.IsEmpty()) {
    return new rtc::RefCountedObject<WrapI420P>(source);
  }

  return nullptr;
}

WrapI420P::WrapI420P(const Let<I420P> &source) :
  _source(source)
{ }

WrapI420P::~WrapI420P() {

}

int WrapI420P::width() const {
  return _source->Width();
}

int WrapI420P::height() const {
  return _source->Height();
}

const uint8_t* WrapI420P::DataY() const {
  return _source->DataY();
}

const uint8_t* WrapI420P::DataU() const {
  return _source->DataU();
}

const uint8_t* WrapI420P::DataV() const {
  return _source->DataV();
}

int WrapI420P::StrideY() const {
  return _source->StrideY();
}

int WrapI420P::StrideU() const {
  return _source->StrideU();
}

int WrapI420P::StrideV() const {
  return _source->StrideV();
}

void* WrapI420P::native_handle() const {
  return nullptr;
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> WrapI420P::NativeToI420Buffer() {
  return nullptr;
}

Let<I420P> WrapVideoFrameBuffer::New(const rtc::scoped_refptr<webrtc::VideoFrameBuffer> &vfb) {
  if (vfb.get()) {
    return Let<WrapVideoFrameBuffer>::New(vfb);
  }

  return Let<I420P>::Empty();
}

WrapVideoFrameBuffer::WrapVideoFrameBuffer(const rtc::scoped_refptr<webrtc::VideoFrameBuffer> &vfb) :
  _vfb(vfb)
{ }

WrapVideoFrameBuffer::~WrapVideoFrameBuffer() {

}

int WrapVideoFrameBuffer::Width() const {
  return _vfb->width();
}

int WrapVideoFrameBuffer::Height() const {
  return _vfb->height();
}

const uint8_t* WrapVideoFrameBuffer::DataY() const {
  return _vfb->DataY();
}

const uint8_t* WrapVideoFrameBuffer::DataU() const {
  return _vfb->DataU();
}

const uint8_t* WrapVideoFrameBuffer::DataV() const {
  return _vfb->DataV();
}

int WrapVideoFrameBuffer::StrideY() const {
  return _vfb->StrideY();
}

int WrapVideoFrameBuffer::StrideU() const {
  return _vfb->StrideU();
}

int WrapVideoFrameBuffer::StrideV() const {
  return _vfb->StrideV();
}

size_t WrapVideoFrameBuffer::ByteLength() const {
  return I420P::ByteLength(_vfb->height(), _vfb->StrideY(), _vfb->StrideU(), _vfb->StrideV());
}

Let<ArrayBuffer> WrapVideoFrameBuffer::Slice(size_t begin, size_t end) const {
  Let<ArrayBuffer> buffer = ArrayBuffer::New(Data(), ByteLength());

  if (!buffer.IsEmpty()) {
    return buffer->Slice(begin, end);
  }

  return Let<ArrayBuffer>::Empty();
}

uint8_t *WrapVideoFrameBuffer::Data() {
  return const_cast<uint8_t*>(_vfb->DataY());
}

const uint8_t *WrapVideoFrameBuffer::Data() const {
  return _vfb->DataY();
}

std::string WrapVideoFrameBuffer::ToString() const {
  return std::string(std::string(reinterpret_cast<const char *>(Data()), ByteLength()));
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> WrapBufferToVideoFrameBuffer::New(const Let<ArrayBuffer> &source, int width, int height) {
  if (!source.IsEmpty()) {
    return new rtc::RefCountedObject<WrapBufferToVideoFrameBuffer>(source, width, height);
  }

  return nullptr;
}

WrapBufferToVideoFrameBuffer::WrapBufferToVideoFrameBuffer(const Let<ArrayBuffer> &source, int width, int height) :
  _source(source),
  _width(width),
  _height(height),
  _y(nullptr),
  _u(nullptr),
  _v(nullptr)
{
  _y = source->Data();
  _u = source->Data() + _width * _height;
  _v = source->Data() + _width * _height + ((_width + 1) >> 1) * ((_height + 1) >> 1);
}

WrapBufferToVideoFrameBuffer::~WrapBufferToVideoFrameBuffer() {

}

int WrapBufferToVideoFrameBuffer::width() const {
  return _width;
}

int WrapBufferToVideoFrameBuffer::height() const {
  return _height;
}

const uint8_t* WrapBufferToVideoFrameBuffer::DataY() const {
  return _y;
}

const uint8_t* WrapBufferToVideoFrameBuffer::DataU() const {
  return _u;
}

const uint8_t* WrapBufferToVideoFrameBuffer::DataV() const {
  return _v;
}

int WrapBufferToVideoFrameBuffer::StrideY() const {
  return _width;
}

int WrapBufferToVideoFrameBuffer::StrideU() const {
  return (_width + 1) >> 1;
}

int WrapBufferToVideoFrameBuffer::StrideV() const {
  return (_width + 1) >> 1;
}

void* WrapBufferToVideoFrameBuffer::native_handle() const {
  return nullptr;
}

rtc::scoped_refptr<webrtc::VideoFrameBuffer> WrapBufferToVideoFrameBuffer::NativeToI420Buffer() {
  return nullptr;
}
