#include <stdio.h>
#include <string>
#include <map>
#include <iostream>

#include "crtc.h"

using namespace crtc;

std::vector<Let<RTCPeerConnection>> peers;
std::vector<Let<RTCDataChannel>> channels;

volatile int open_peers = 0;
volatile int closed_peers = 0;
volatile int open_channels = 0;
volatile int closed_channels = 0;
volatile int send_ping = 0;
volatile int recv_ping = 0;
volatile int send_pong = 0;
volatile int recv_pong = 0;
volatile int messages_in = 0;
volatile int messages_out = 0;

void RemovePeerEvents(const Let<RTCPeerConnection> &peer) {
  peer->onsignalingstatechange.Dispose();
  peer->onicegatheringstatechange.Dispose();
  peer->oniceconnectionstatechange.Dispose();
  peer->onicecandidatesremoved.Dispose();
  peer->onaddstream.Dispose();
  peer->onremovestream.Dispose();
  peer->ondatachannel.Dispose();
  peer->onicecandidate.Dispose();
}

void RemoveChannelEvents(const Let<RTCDataChannel> &dc) {
  dc->onopen.Dispose();
  dc->onclose.Dispose();
  dc->onerror.Dispose();
  dc->onmessage.Dispose();
}

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
        Atomic::Increment(&closed_peers);

        SetImmediate([=]() {
          RemovePeerEvents(ls);
        });

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
    dataChannel->onopen = [=]() {
      Atomic::Increment(&open_channels);
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Opened" << std::endl;

      for (int index = 0; index < 10; index++) {
        dataChannel->Send(ArrayBuffer::New("PING"));
        Atomic::Increment(&send_ping);
        Atomic::Increment(&messages_out);
      }      
    };

    dataChannel->onclose = [=]() {
      SetImmediate([=]() {
        RemoveChannelEvents(dataChannel);
      });

      Atomic::Increment(&closed_channels);
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Closed" << std::endl;
    };

    dataChannel->onerror = [=](const Let<Error> &error) {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: " << error->ToString() << std::endl;
    };

    dataChannel->onmessage = [=](const Let<ArrayBuffer> &buffer, bool binary) {
      std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: Message" << std::endl;
      Atomic::Increment(&messages_in);

      if (!buffer->ToString().compare("PING")) {
        Atomic::Increment(&recv_ping);
        std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: PING" << std::endl;
        dataChannel->Send(ArrayBuffer::New("PONG"));
        Atomic::Increment(&send_pong);
        Atomic::Increment(&messages_out);
      } else if (!buffer->ToString().compare("PONG")) {
        Atomic::Increment(&recv_pong);
        std::cout << left << " ==> " << right << " [DataChannel: " << dataChannel->Id() << ", Label: " << dataChannel->Label() << "]: PONG" << std::endl;
      }
    };

    switch (dataChannel->ReadyState()) {
      case RTCDataChannel::State::kConnecting:
        std::cout << left << " ==> " << right << " [DataChannel: Connecting" << std::endl;
        break;
      case RTCDataChannel::State::kOpen:
        std::cout << left << " ==> " << right << " [DataChannel: Open" << std::endl;
        dataChannel->onopen();
        break;
      case RTCDataChannel::State::kClosing:
        std::cout << left << " ==> " << right << " [DataChannel: Closing" << std::endl;
        break;
      case RTCDataChannel::State::kClosed:
        std::cout << left << " ==> " << right << " [DataChannel: Closed" << std::endl;
        dataChannel->onclose();
        break;
    }

    channels.push_back(dataChannel);
  };

  ls->onicecandidate = [=](const RTCPeerConnection::RTCIceCandidate &iceCandidate) {
    std::cout << left << " <-> " << right << " [OnIceCandidate]" << std::endl;

    rs->AddIceCandidate(iceCandidate)->Then([=]() {
      std::cout << left << " <-> " << right << " [AddIceCandidate]: Done!" << std::endl;
    })->Catch([=](const Let<Error> &error) {
      std::cout << left << " <-> " << right << " [AddIceCandidate]: " << error->ToString() << std::endl;
    });
  };
}

void CreatePeerPair(const std::string &left, const std::string &right) {
  std::cout << left << " <-> " << right << " [ " << peers.size() << " ] " << std::endl;

  Let<RTCPeerConnection> ls = RTCPeerConnection::New();
  Let<RTCPeerConnection> rs = RTCPeerConnection::New();

  makePair(left, right, ls, rs);
  makePair(right, left, rs, ls);

  for (int index = 0; index < 10; index++) {
    ls->ondatachannel(ls->CreateDataChannel(left));
  }

  open_peers += 2;

  peers.push_back(ls);
  peers.push_back(rs);
}

void ClosePeers() {
  std::cout << "Closing " << peers.size() << " peers..." << std::endl;

  for (auto peer : peers) {
    peer->Close();
  }
}

const char *names[] = {
  "joshua",
  "alec",
  "kathy",
  //"kyle",
  //"mary",
  //"harry",
  //"candice",
  //"kelly",
  //"jesse",
  //"edward",
  //"george",
  //"albert",
  //"alice",
  //"bob",
  //"stuart",
};

int main() {
  Module::Init();

  size_t peer_count = (sizeof(names) / sizeof(const char *));
  size_t expected_count = peer_count * peer_count - peer_count;

  std::cout << "Creating Mesh Network with " << peer_count << " peers." << std::endl;

  for (size_t x = 0; x < peer_count; x++) {
    for (size_t y = x + 1; y < peer_count; y++) {
      CreatePeerPair(names[x], names[y]);
    }
  }

  SetTimeout(&ClosePeers, 15000);

  Module::DispatchEvents(true);
  Module::Dispose();

  std::cout << "Peers created: " << Atomic::AcquireLoad(&open_peers) << " (expected: " << expected_count << ")" << std::endl;
  std::cout << "Peers closed: " << Atomic::AcquireLoad(&closed_peers) << " (expected: " << expected_count << ")" << std::endl;
  std::cout << "Datachannels opened: " << Atomic::AcquireLoad(&open_channels) << " (expected: " << expected_count * 10 << ")" << std::endl;
  std::cout << "Datachannels closed: " << Atomic::AcquireLoad(&closed_channels) << " (expected: " << expected_count * 10 << ")" << std::endl;
  std::cout << "Messages received: " << Atomic::AcquireLoad(&messages_in) << " (expected: " << expected_count * 10 * 10 * 2 << ")" << std::endl;
  std::cout << "Messages sended: " << Atomic::AcquireLoad(&messages_out) << " (expected: " << expected_count * 10 * 10 * 2 << ")" << std::endl;
  std::cout << "Ping send count: " << Atomic::AcquireLoad(&send_ping) << " (expected: " << expected_count * 10 * 10 << ")" << std::endl;
  std::cout << "Ping received count: " << Atomic::AcquireLoad(&recv_ping) << " (expected: " << expected_count * 10 * 10 << ")" << std::endl;
  std::cout << "Pong send count: " << Atomic::AcquireLoad(&send_pong) << " (expected: " << expected_count * 10 * 10 << ")" << std::endl;
  std::cout << "Pong received count: " << Atomic::AcquireLoad(&recv_pong) << " (expected: " << expected_count * 10 * 10 << ")" << std::endl;

  return 0;
}