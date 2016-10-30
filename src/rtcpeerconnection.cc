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
#include "rtcpeerconnection.h"

using namespace crtc;

IceServer::IceServer() {

}

IceServer::~IceServer() {

}

RTCPeerConnectionInternal::Socket::Socket() {

}

RTCPeerConnectionInternal::Socket::~Socket() {
  
}

void RTCPeerConnectionInternal::Socket::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {

}

void RTCPeerConnectionInternal::Socket::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {

}

void RTCPeerConnectionInternal::Socket::OnAddStream(webrtc::MediaStreamInterface* stream) {

}

void RTCPeerConnectionInternal::Socket::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {

}

void RTCPeerConnectionInternal::Socket::OnRemoveStream(webrtc::MediaStreamInterface* stream) {

}

void RTCPeerConnectionInternal::Socket::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

}

void RTCPeerConnectionInternal::Socket::OnDataChannel(webrtc::DataChannelInterface* data_channel) {

}

void RTCPeerConnectionInternal::Socket::OnRenegotiationNeeded() {

}

void RTCPeerConnectionInternal::Socket::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {

}

void RTCPeerConnectionInternal::Socket::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {

}

void RTCPeerConnectionInternal::Socket::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {

}

void RTCPeerConnectionInternal::Socket::OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) {

}

void RTCPeerConnectionInternal::Socket::OnIceConnectionReceivingChange(bool receiving) {
  
}

RTCPeerConnectionInternal::RTCPeerConnectionInternal(const RTCConfiguration &config) {
  
}

RTCPeerConnectionInternal::~RTCPeerConnectionInternal() {

}

