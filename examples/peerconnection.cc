#include <stdio.h>
#include <string>
#include <map>
#include <iostream>

#include "crtc.h"

using namespace crtc;

void makePair(const std::string &left, const std::string &right,
              const Let<RTCPeerConnection> &ls, const Let<RTCPeerConnection> &rs) 
{
  ls->onnegotiationneeded = [=]() {
    std::cout << left << " <-> " << right << " [OnNegotiationNeeded]" << std::endl;

    ls->CreateOffer()->Then([=](const RTCPeerConnection::RTCSessionDescription &offer) {
      ls->SetLocalDescription(offer)->Then([=]() {
        rs->SetRemoteDescription(offer)->Then([=]() {
          rs->CreateAnswer()->Then([=](const RTCPeerConnection::RTCSessionDescription &answer) {
            rs->SetLocalDescription(answer)->Then([=]() {
              ls->SetRemoteDescription(answer)->Then([=]() {
                std::cout << left << " <-> " << right << " [OnNegotiationNeeded]: Done!" << std::endl;
              })->Catch([=](const Let<Error> &error) {
                std::cout << left << " <-> " << right << " [SetRemoteDescription]: " << error->ToString() << std::endl;
              });
            })->Catch([=](const Let<Error> &error) {
              std::cout << right << " <-> " << left << " [SetLocalDescription]: " << error->ToString() << std::endl;
            });
          })->Catch([=](const Let<Error> &error) {
            std::cout << right << " <-> " << left << " [CreateAnswer]: " << error->ToString() << std::endl;
          });
        })->Catch([=](const Let<Error> &error) {
          std::cout << right << " <-> " << left << " [SetRemoteDescription]: " << error->ToString() << std::endl;
        });
      })->Catch([=](const Let<Error> &error) {
        std::cout << left << " <-> " << right << " [SetLocalDescription]: " << error->ToString() << std::endl;
      });
    })->Catch([=](const Let<Error> &error) {
      std::cout << left << " <-> " << right << " [CreateOffer]: " << error->ToString() << std::endl;
    });
  };

  ls->onsignalingstatechange = [=]() {
    switch (ls->SignalingState()) {
      case RTCPeerConnection::kStable:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: stable" << std::endl;
        break;
      case RTCPeerConnection::kHaveLocalOffer:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: have-local-offer" << std::endl;
        break;
      case RTCPeerConnection::kHaveLocalPrAnswer:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: have-local-pranswer" << std::endl;
        break;
      case RTCPeerConnection::kHaveRemoteOffer:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: have-remote-offer" << std::endl;
        break;
      case RTCPeerConnection::kHaveRemotePrAnswer:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: have-remote-pranswer" << std::endl;
        break;
      case RTCPeerConnection::kSignalingClosed:
        std::cout << left << " <-> " << right << " [OnSignalingStateChange]: closed" << std::endl;
        break;
    }
  };

  ls->onicegatheringstatechange = [=]() {
    switch (ls->IceGatheringState()) {
      case RTCPeerConnection::kNewGathering:
        std::cout << left << " <-> " << right << " [OnIceGatheringStateChange]: new" << std::endl;
        break;
      case RTCPeerConnection::kGathering:
        std::cout << left << " <-> " << right << " [OnIceGatheringStateChange]: gathering" << std::endl;
        break;
      case RTCPeerConnection::kComplete:
        std::cout << left << " <-> " << right << " [OnIceGatheringStateChange]: complete" << std::endl;
        break;
    }
  };

  ls->oniceconnectionstatechange = [=]() {
    switch (ls->IceConnectionState()) {
      case RTCPeerConnection::kNew:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: new" << std::endl;
        break;
      case RTCPeerConnection::kChecking:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: checking" << std::endl;
        break;
      case RTCPeerConnection::kConnected:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: connected" << std::endl;
        break;
      case RTCPeerConnection::kCompleted:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: completed" << std::endl;
        break;
      case RTCPeerConnection::kFailed:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: cailed" << std::endl;
        break;
      case RTCPeerConnection::kDisconnected:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: disconnected" << std::endl;
        break;
      case RTCPeerConnection::kClosed:
        std::cout << left << " <-> " << right << " [OnIceConnectionStateChange]: closed" << std::endl;
        break;
    }
  };

  ls->onicecandidatesremoved = [=]() {
    std::cout << left << " <-> " << right << " [OnIceCandidatesRemoved]" << std::endl;
  };

  ls->onaddstream = [=](const Let<MediaStream> &stream) {
    std::cout << left << " <-> " << right << " [OnAddStream]" << std::endl;
  };

  ls->onremovestream = [=](const Let<MediaStream> &stream) {
    std::cout << left << " <-> " << right << " [OnRemoveStream]" << std::endl;
  };

  ls->ondatachannel = [=](const Let<RTCDataChannel> &dataChannel) {
    std::cout << left << " ==> " << right << " [OnDataChannel]" << std::endl;

    dataChannel->onopen = [=]() {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Opened" << std::endl;
      dataChannel->Send(ArrayBuffer::New("PING"));
    };

    dataChannel->onclose = [=]() {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Closed" << std::endl;
    };

    dataChannel->onerror = [=](const Let<Error> &error) {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: " << error->ToString() << std::endl;
    };

    dataChannel->onmessage = [=](const Let<ArrayBuffer> &buffer, bool binary) {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Message" << std::endl;

      if (!buffer->ToString().compare("PING")) {
        std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: PING" << std::endl;
        dataChannel->Send(ArrayBuffer::New("PONG"));
      } else if (!buffer->ToString().compare("PONG")) {
        std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: PONG" << std::endl;
      }
    };

    switch (dataChannel->ReadyState()) {
      case RTCDataChannel::kConnecting:
        std::cout << left << " ==> " << right << " [DataChannel: Connecting" << std::endl;
        break;
      case RTCDataChannel::kOpen:
        std::cout << left << " ==> " << right << " [DataChannel: Open" << std::endl;
        dataChannel->onopen();
        break;
      case RTCDataChannel::kClosing:
        std::cout << left << " ==> " << right << " [DataChannel: Closing" << std::endl;
        break;
      case RTCDataChannel::kClosed:
        std::cout << left << " ==> " << right << " [DataChannel: Closed" << std::endl;
        dataChannel->onclose();
        break;
    }
  };

  ls->onicecandidate = [=](const RTCPeerConnection::RTCIceCandidate &candidate) {
    std::cout << left << " <-> " << right << " [OnIceCandidate]" << std::endl;

    rs->AddIceCandidate(candidate)->Then([=]() {
      std::cout << left << " <-> " << right << " [AddIceCandidate]: Done!" << std::endl;
    })->Catch([=](const Let<Error> &error) {
      std::cout << left << " <-> " << right << " [AddIceCandidate]: " << error->ToString() << std::endl;
    });
  };
}

int main() {
  Module::Init();

  Let<RTCPeerConnection> alice = RTCPeerConnection::New();
  Let<RTCPeerConnection> bob = RTCPeerConnection::New();

  makePair("alice", "bob", alice, bob);
  makePair("bob", "alice", bob, alice);

  Let<RTCDataChannel> dataChannel = alice->CreateDataChannel("datachannel");

  if (!dataChannel.IsEmpty()) {
    alice->ondatachannel(dataChannel);
  }
  
  SetTimeout([=]() {
    alice->Close();
    bob->Close();
  }, 30000);

  Module::DispatchEvents(true);
  Module::Dispose();

  return 0;
}