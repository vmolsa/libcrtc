
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

#ifndef CRTC_RTCPEERCONNECTION_H
#define CRTC_RTCPEERCONNECTION_H

#include "crtc.h"
#include "event.h"

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/media/engine/webrtcvideodecoderfactory.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include "webrtc/modules/audio_device/include/audio_device.h"

namespace crtc {
  class RTCPeerConnectionInternal;

  class RTCPeerConnectionInternal : public RTCPeerConnection, public webrtc::PeerConnectionObserver {
      friend class Let<RTCPeerConnectionInternal>;
      friend class RTCPeerConnectionObserver;

    public:
      static void Init();
      static void Dispose();
      
      static std::unique_ptr<rtc::Thread> network_thread;
      static std::unique_ptr<rtc::Thread> worker_thread;
      static rtc::scoped_refptr<webrtc::AudioDeviceModule> audio_device;
      static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;

      Let<RTCDataChannel> CreateDataChannel(const std::string &label, const RTCDataChannelInit &options = RTCDataChannelInit()) override;
      Let<Promise<void>> AddIceCandidate(const RTCPeerConnection::RTCIceCandidate &candidate) override;
      void AddStream(const Let<MediaStream> &stream) override;
      // Let<RTCPeerConnection::RTCRtpSender> AddTrack(const Let<MediaStreamTrack> &track, const Let<MediaStream> &stream) override;
      Let<Promise<RTCPeerConnection::RTCSessionDescription>> CreateAnswer(const RTCPeerConnection::RTCAnswerOptions &options) override;
      Let<Promise<RTCPeerConnection::RTCSessionDescription>> CreateOffer(const RTCPeerConnection::RTCOfferOptions &options) override;
      // Let<Promise<RTCPeerConnection::RTCCertificate>> GenerateCertificate() override;
      MediaStreams GetLocalStreams() override;
      MediaStreams GetRemoteStreams() override;
      void RemoveStream(const Let<MediaStream> &stream) override;
      // void RemoveTrack(const Let<RTCPeerConnection::RTCRtpSender> &sender) override;
      void SetConfiguration(const RTCPeerConnection::RTCConfiguration &config) override;
      Let<Promise<void>> SetLocalDescription(const RTCPeerConnection::RTCSessionDescription &sdp) override;
      Let<Promise<void>> SetRemoteDescription(const RTCPeerConnection::RTCSessionDescription &sdp) override;
      void Close() override;

      RTCPeerConnection::RTCSessionDescription CurrentLocalDescription() override;
      RTCPeerConnection::RTCSessionDescription CurrentRemoteDescription() override;
      RTCPeerConnection::RTCSessionDescription LocalDescription() override;
      RTCPeerConnection::RTCSessionDescription PendingLocalDescription() override;
      RTCPeerConnection::RTCSessionDescription PendingRemoteDescription() override;
      RTCPeerConnection::RTCSessionDescription RemoteDescription() override; 

      RTCPeerConnection::RTCIceConnectionState IceConnectionState() override;
      RTCPeerConnection::RTCIceGatheringState IceGatheringState() override;
      RTCPeerConnection::RTCSignalingState SignalingState() override;

    private:
      inline static Let<Error> SDP2SDP(const webrtc::SessionDescriptionInterface* desc = nullptr, RTCPeerConnection::RTCSessionDescription *sdp = nullptr) {
        if (desc && sdp) {
          if (desc->type().compare(webrtc::SessionDescriptionInterface::kOffer) == 0) {
            sdp->type = RTCPeerConnection::RTCSessionDescription::kOffer;
          } else if (desc->type().compare(webrtc::SessionDescriptionInterface::kAnswer) == 0) {
            sdp->type = RTCPeerConnection::RTCSessionDescription::kAnswer;
          } else {
            sdp->type = RTCPeerConnection::RTCSessionDescription::kPranswer;
          }
          
          if (desc->ToString(&sdp->sdp)) {
            return Let<Error>();
          }

          return Error::New("Unable to create SessionDescription", __FILE__, __LINE__);
        }

        return Error::New("Invalid SessionDescriptionInterface", __FILE__, __LINE__);
      }

      inline static Let<Error> SDP2SDP(const RTCPeerConnection::RTCSessionDescription &sdp, webrtc::SessionDescriptionInterface **desc = nullptr) {
        std::string type;
        webrtc::SdpParseError error;

        if (desc) {
          switch (sdp.type) {
            case RTCPeerConnection::RTCSessionDescription::kAnswer:
              type = webrtc::SessionDescriptionInterface::kAnswer;
              break;
            case RTCPeerConnection::RTCSessionDescription::kOffer:
              type = webrtc::SessionDescriptionInterface::kOffer;
              break;
            case RTCPeerConnection::RTCSessionDescription::kPranswer:
              type = webrtc::SessionDescriptionInterface::kPrAnswer;
              break;
            case RTCPeerConnection::RTCSessionDescription::kRollback:
              break; 
          }

          *desc = webrtc::CreateSessionDescription(type, sdp.sdp, &error);

          if (*desc) {
            return Let<Error>();
          } else {
            return Error::New(error.description, __FILE__, __LINE__);
          }
        } else {
          return Error::New("Invalid SessionDescriptionInterface", __FILE__, __LINE__);
        }            
      }

      inline static Let<Error> ParseConfiguration(
        const RTCPeerConnection::RTCConfiguration &config,
        webrtc::PeerConnectionInterface::RTCConfiguration *cfg = nullptr) 
      {
        if (cfg) {
          // cfg->certificates = config.certificates;

          cfg->ice_candidate_pool_size = config.iceCandidatePoolSize;
          
          switch (config.iceTransportPolicy) {
            case RTCPeerConnection::kRelay:
              cfg->type = webrtc::PeerConnectionInterface::kRelay;
              break;
            case RTCPeerConnection::kPublic:
              cfg->type = webrtc::PeerConnectionInterface::kNoHost;
              break;
            case RTCPeerConnection::kAll:
              cfg->type = webrtc::PeerConnectionInterface::kAll;
              break;
          }

          switch (config.rtcpMuxPolicy) {
            case RTCPeerConnection::kNegotiate:
              cfg->rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyNegotiate;
              break;
            case RTCPeerConnection::kRequire:
              cfg->rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;
              break;
          }

          switch (config.bundlePolicy) {
            case RTCPeerConnection::kBalanced:
              cfg->bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyBalanced;
              break;
            case RTCPeerConnection::kMaxBundle:
              cfg->bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
              break;
            case RTCPeerConnection::kMaxCompat:
              cfg->bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat;
              break;
          }

          for (const auto &iceserver: config.iceServers) {
            webrtc::PeerConnectionInterface::IceServer server;

            server.urls = iceserver.urls;
            server.username = iceserver.username;
            server.password = iceserver.credential;

            cfg->servers.push_back(server);
          }

          return Let<Error>();
        }

        return Error::New("Invalid RTCConfiguration", __FILE__, __LINE__);
      }

      class CreateOfferAnswerObserver : public webrtc::CreateSessionDescriptionObserver {
        public:
          CreateOfferAnswerObserver(const Promise<RTCPeerConnection::RTCSessionDescription>::FullFilledCallback &resolve,
                                    const Promise<RTCPeerConnection::RTCSessionDescription>::RejectedCallback &reject) :
            _resolve(resolve),
            _reject(reject)
          { }

          ~CreateOfferAnswerObserver() override { }

        private:
          void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
            RTCPeerConnection::RTCSessionDescription sdp;

            Let<Error> error = SDP2SDP(desc, &sdp);

            if (error.IsEmpty()) {
              _resolve(sdp);
            } else {
              _reject(error);
            }
          }

          void OnFailure(const std::string& error) override {
            _reject(Error::New(error, __FILE__, __LINE__));
          }

          Promise<RTCPeerConnection::RTCSessionDescription>::FullFilledCallback _resolve;
          Promise<RTCPeerConnection::RTCSessionDescription>::RejectedCallback _reject;
      };

      class SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
        public:
          SetSessionDescriptionObserver(const Promise<void>::FullFilledCallback &resolve,
                                        const Promise<void>::RejectedCallback &reject) :
            _resolve(resolve),
            _reject(reject)
          { }

          ~SetSessionDescriptionObserver() override { }

        private:
          void OnSuccess() override {
            _resolve();
          }

          void OnFailure(const std::string& error) override {
            _reject(Error::New(error, __FILE__, __LINE__));
          }

          Promise<void>::FullFilledCallback _resolve;
          Promise<void>::RejectedCallback _reject;
      };

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

    protected:
      explicit RTCPeerConnectionInternal(const RTCConfiguration &config = RTCConfiguration());
      ~RTCPeerConnectionInternal() override;

      rtc::scoped_refptr<webrtc::PeerConnectionInterface> _socket;
      rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;
      Let<Event> _event;
      std::vector<Callback> _pending_candidates;
  };
};

#endif