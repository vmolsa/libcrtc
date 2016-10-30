
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

#ifndef CRTC_RTCPEERCONNECTION_H
#define CRTC_RTCPEERCONNECTION_H

#include "crtc.h"

#include "webrtc/api/peerconnectioninterface.h"

namespace crtc {
  class RTCPeerConnectionInternal : public RTCPeerConnection {
    public:
      class Socket : public webrtc::PeerConnectionInterface, public webrtc::PeerConnectionObserver  {
        public: 
          Socket();
          ~Socket() override;

          void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
          void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
          void OnAddStream(webrtc::MediaStreamInterface* stream) override;
          void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
          void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;
          void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
          void OnDataChannel(webrtc::DataChannelInterface* data_channel) override;
          void OnRenegotiationNeeded() override;
          void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
          void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
          void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
          void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override;
          void OnIceConnectionReceivingChange(bool receiving) override;
      };

      explicit RTCPeerConnectionInternal(const RTCConfiguration &config);
      ~RTCPeerConnectionInternal() override;


    protected:
      
  };
};

#endif