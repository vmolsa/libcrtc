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
#include <cstring>

#include "arraybuffer.h"

using namespace crtc;

Let<ArrayBuffer> ArrayBuffer::New(const std::string &data) {
  return Let<ArrayBufferInternal>::New(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

Let<ArrayBuffer> ArrayBuffer::New(size_t byteLength) {
  return Let<ArrayBufferInternal>::New(nullptr, byteLength);
}

Let<ArrayBuffer> ArrayBuffer::New(const uint8_t *data, size_t byteLength) {
  return Let<ArrayBufferInternal>::New(data, byteLength);
}

ArrayBufferInternal::ArrayBufferInternal(const uint8_t *data, size_t byteLength) : 
  _alloc(false),
  _data(nullptr),
  _byteLength(0)
{
  ArrayBufferInternal::Init(data, byteLength);
}

ArrayBufferInternal::ArrayBufferInternal(const Let<ArrayBuffer> &buffer) : 
  _alloc(false),
  _data(nullptr),
  _byteLength(0)
{
  if (!buffer.IsEmpty()) {
    ArrayBufferInternal::Init(buffer->Data(), buffer->ByteLength());
  } 
}

ArrayBufferInternal::~ArrayBufferInternal() {
  if (_alloc && _data) {
    delete [] _data;
  }
}

void ArrayBufferInternal::Init(const uint8_t *data, size_t byteLength) {
  if (byteLength) {
    _data = new uint8_t[byteLength];
    _byteLength = byteLength;
    _alloc = true;

    if (data != nullptr) {
      std::memcpy(_data, data, _byteLength);
    } else {
      std::memset(_data, 0, _byteLength);
    }
  }
}

size_t ArrayBufferInternal::ByteLength() const {
  return _byteLength;
}

Let<ArrayBuffer> ArrayBufferInternal::Slice(size_t begin, size_t end) const {
  if (begin <= end && end <= _byteLength) {
    return Let<ArrayBufferInternal>::New(_data + begin, ((!end) ? _byteLength : end - begin));
  }

  return Let<ArrayBuffer>();
}

uint8_t *ArrayBufferInternal::Data() {
  return _data;
}

const uint8_t *ArrayBufferInternal::Data() const {
  return _data;
}

std::string ArrayBufferInternal::ToString() const {
  return std::string(reinterpret_cast<const char *>(_data), _byteLength);
}