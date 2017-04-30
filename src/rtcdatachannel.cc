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
#include "rtcdatachannel.h"
#include "arraybuffer.h"

using namespace crtc;

RTCDataChannelInternal::RTCDataChannelInternal(const rtc::scoped_refptr<webrtc::DataChannelInterface> &channel) :
  _threshold(0),
  _channel(channel)
{
  _channel->RegisterObserver(this);

  if (_channel->state() == webrtc::DataChannelInterface::kOpen || 
      _channel->state() == webrtc::DataChannelInterface::kConnecting) 
  {
    _event = Event::New();
  }
}

RTCDataChannelInternal::~RTCDataChannelInternal() {
  _channel->UnregisterObserver();
}

int RTCDataChannelInternal::Id() {
  return _channel->id();
}

std::string RTCDataChannelInternal::Label() {
  return _channel->label();
}

uint64_t RTCDataChannelInternal::BufferedAmount() {
  return _channel->buffered_amount();
}

uint64_t RTCDataChannelInternal::BufferedAmountLowThreshold() {
  return _threshold;
}

void RTCDataChannelInternal::SetBufferedAmountLowThreshold(uint64_t threshold) {
  _threshold = threshold;
}

uint16_t RTCDataChannelInternal::MaxPacketLifeTime() {
  return _channel->maxRetransmitTime();
}

uint16_t RTCDataChannelInternal::MaxRetransmits() {
  return _channel->maxRetransmits();
}

bool RTCDataChannelInternal::Negotiated() {
  return _channel->negotiated();
}

bool RTCDataChannelInternal::Ordered() {
  return _channel->ordered();
}

std::string RTCDataChannelInternal::Protocol() {
  return _channel->protocol();
}

RTCDataChannel::State RTCDataChannelInternal::ReadyState() {
  switch (_channel->state()) {
    case webrtc::DataChannelInterface::kConnecting:
      return RTCDataChannel::State::kConnecting;
    case webrtc::DataChannelInterface::kOpen:
      return RTCDataChannel::State::kOpen;
    case webrtc::DataChannelInterface::kClosing:
      return RTCDataChannel::State::kClosing;
    case webrtc::DataChannelInterface::kClosed:
      return RTCDataChannel::State::kClosed;
  }
}

void RTCDataChannelInternal::Close() {
  _channel->Close();
}

void RTCDataChannelInternal::Send(const Let<ArrayBuffer> &data, bool binary) {
  rtc::CopyOnWriteBuffer buffer(data->Data(), data->ByteLength());
  webrtc::DataBuffer dataBuffer(buffer, binary);

  if (!_channel->Send(dataBuffer)) {
    switch (_channel->state()) {
      case webrtc::DataChannelInterface::kConnecting:
        onerror(Error::New("Unable to send arraybuffer. DataChannel is connecting", __FILE__, __LINE__));
        break;
      case webrtc::DataChannelInterface::kOpen:
        onerror(Error::New("Unable to send arraybuffer.", __FILE__, __LINE__));
        break;
      case webrtc::DataChannelInterface::kClosing:
        onerror(Error::New("Unable to send arraybuffer. DataChannel is closing", __FILE__, __LINE__));
        break;
      case webrtc::DataChannelInterface::kClosed:
        onerror(Error::New("Unable to send arraybuffer. DataChannel is closed", __FILE__, __LINE__));
        break;
    }
  }
}

void RTCDataChannelInternal::OnStateChange() {
  switch (_channel->state()) {
    case webrtc::DataChannelInterface::kConnecting:
      break;
    case webrtc::DataChannelInterface::kOpen:
      onopen();
      break;
    case webrtc::DataChannelInterface::kClosing:
      break;
    case webrtc::DataChannelInterface::kClosed:
      onclose();
      _event.Dispose();
      break;
  }
}

void RTCDataChannelInternal::OnMessage(const webrtc::DataBuffer& buffer) {
  onmessage(Let<WrapRtcBuffer>::New(buffer.data), buffer.binary);
}

void RTCDataChannelInternal::OnBufferedAmountChange(uint64_t previous_amount) {
  if (_threshold && previous_amount > _threshold && _channel->buffered_amount() < _threshold) {
    onbufferedamountlow();
  }
}

WrapRtcBuffer::WrapRtcBuffer(const rtc::CopyOnWriteBuffer &buffer) : _data(buffer) {

}

WrapRtcBuffer::~WrapRtcBuffer() {

}

size_t WrapRtcBuffer::ByteLength() const {
  return _data.size();
}

Let<ArrayBuffer> WrapRtcBuffer::Slice(size_t begin, size_t end) const {
  if (begin <= end && end <= _data.size()) {
    return Let<ArrayBufferInternal>::New(_data.data() + begin, ((!end) ? _data.size() : end - begin));
  }

  return Let<ArrayBuffer>();
}

uint8_t *WrapRtcBuffer::Data() {
  return _data.data();
}

const uint8_t *WrapRtcBuffer::Data() const {
  return _data.data();
}

std::string WrapRtcBuffer::ToString() const {
  return std::string(reinterpret_cast<const char *>(_data.data()), _data.size());
}

RTCDataChannel::RTCDataChannel() {

}

RTCDataChannel::~RTCDataChannel() {
  
}