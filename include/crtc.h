
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

#ifndef CRTC_H
#define CRTC_H

#include <memory>
#include <queue>
#include <vector>
#include <string>

#ifdef CRTC_OS_WIN
  #define CRTC_EXPORT __declspec(dllexport)
  #define CRTC_NO_EXPORT __declspec(dllimport)
#else
  #define CRTC_EXPORT __attribute__((visibility("default")))
  #define CRTC_NO_EXPORT __attribute__((visibility("hidden")))
#endif

namespace crtc {
  class CRTC_EXPORT Reference {
    public:
      virtual void Increment() const = 0;
      virtual void Decrement() const = 0;

    protected:
      virtual ~Reference() {}
  };

  template <class T> class CRTC_EXPORT Let {
    public:
      #ifdef CRTC_OS_WIN
        class Constructor : public T {
          public:
            template <class A0> explicit Constructor(A0 &&arg) : T(std::forward<A0>(arg)) {}

            inline virtual void Increment() const {
              ::InterlockedIncrement(&_ref_count);
            }

            inline virtual void Decrement() const {
              if (!::InterlockedDecrement(&_ref_count)) {
                delete this;
              }
            }
          protected:
            mutable volatile LONG _ref_count = 0;
        };
      #else
        class Constructor : public T {
          public:
            template <class A0> explicit Constructor(A0 &&arg) : T(std::forward<A0>(arg)) {}

            inline virtual void Increment() const {
              __sync_add_and_fetch(&_ref_count, 1);
            }

            inline virtual void Decrement() const {
              if (!__sync_sub_and_fetch(&_ref_count, 1)) {
                delete this;
              }
            }
          protected:
            mutable volatile int _ref_count = 0;
        };
      #endif

      template <typename... Args> inline static T* New(Args&&... args) {
        return new Let<T>::Constructor(std::forward<Args>(args)...);
      }

      inline explicit Let() : _ptr(nullptr) {
        
      }

      inline ~Let() {
        Let::Decrement();
      }

      inline Let(T* ptr) : _ptr(ptr) {
        Let::Increment();
      }

      inline Let(const Let<T> &src) : _ptr(*src) {
        Let::Increment();
      }

      template <class S> inline Let(Let<S> src) : _ptr(reinterpret_cast<T*>(*src)) {
        Let::Increment();
      }

      inline Let<T> &operator=(T* src) {
        if (src) { src->Increment(); }

        Let::Decrement();

        _ptr = src;
        return *this;
      }

      template <class S> inline static Let<T> Cast(S* src) {
        return Let<T>(static_cast<T*>(src));
      }

      template <class S> inline static Let<T> Cast(const Let<S> &src) {
        return Let<T>(reinterpret_cast<T*>(*src));
      }

      inline Let<T> &operator=(const Let<T> &src) { return *this = src._ptr; }
      inline bool IsEmpty() const { return (_ptr == nullptr); }
      inline operator T* () const { return _ptr; }
      inline T* operator*() const { return _ptr; }
      inline T* operator->() const { return _ptr; }

      inline void Dispose() {
        Let::Decrement();
        _ptr = nullptr;
      }

    private:
      inline void Increment() {
        if (_ptr) {
          _ptr->Increment();
        }
      }

      inline void Decrement() {
        if (_ptr) {
          _ptr->Decrement();
        }
      }

    protected:
      T* _ptr;
  };

  template <typename T> class Functor;
  template <typename R, typename... Args> class Functor<R(Args...)> {
    public:
      explicit Functor() {}
      virtual ~Functor() {}

      template <class T> inline Functor(const T& functor) : _callback(Let<Wrap<T> >::New(functor)) {}

      inline R operator()(Args&&... args) const {
        if (!_callback.IsEmpty()) {
          return _callback->Call(std::forward<Args>(args)...);
        }

        return R();
      }

    private:
      class Callback : public Reference {
        public:
          ~Callback() override {}
          virtual R Call(Args&&... args) const = 0;
      };

      template <class T> class Wrap : public Callback {
        public:
          explicit Wrap(const T& functor) : _functor(functor) {}

          ~Wrap() override {}

          inline virtual R Call(Args&&... args) const {
            return _functor(std::forward<Args>(args)...);
          }

        protected:
          T _functor;
      };

      Let<Callback> _callback;
  };

  class CRTC_EXPORT Events {
    public:
      static void Init();
      static bool DispatchEvents(bool forever = false);
      static void Dispose();
  };

  class CRTC_EXPORT Event : public Reference {
    public:
      explicit Event();

      Event(const Event& that) = delete;
      Event& operator=(Event const&) = delete;

      ~Event() override;
  };

  class CRTC_EXPORT Async {
    public:
      static void Call(Functor<void()> callback);
  };

  template <typename R = void> class Deferred;

  template <typename R = void> class Promise : public Reference {
      friend class Deferred<R>;
      friend class Let<Promise<R> >;

    public:
      template<typename T> inline Promise* Then(T&& callback) {
        _resolve.push(Functor<void(R)>(callback));
        return this;
      }

      template<typename T> inline Promise* Catch(T&& callback) {
        _reject.push(Functor<void(const std::exception &)>(callback));
        return this;
      }

      template<typename T> inline Promise* Finally(T&& callback) {
        _finally.push(Functor<void()>(callback));
        return this;
      }

      template<typename T> inline static Let<Promise<R> > New(T&& callback) {
        return Let<Promise<R> >::New(std::forward<T>(callback));
      }

    private:
      template<typename T> explicit Promise(T&& callback) {
        Async::Call([this, callback]() {
          Let<Deferred<R> > Q = Let<Deferred<R> >::New(this);
          callback(Q);
        });
      }

      ~Promise() override {}

      inline void Finally() {
        while (!_finally.empty()) {
          Functor<void()> callback(_finally.front());
          callback();
          _finally.pop();
        }
      }

      std::queue<Functor<void(R)> > _resolve;
      std::queue<Functor<void(const std::exception &)> > _reject;
      std::queue<Functor<void()> > _finally;

    protected:
      inline void Resolve(R&& result) {
        while (!_resolve.empty()) {
          Functor<void(R)> callback(_resolve.front());
          callback(std::forward<R>(result));
          _resolve.pop();
        }

        Finally();
      }

      inline void Reject(const std::exception &exception) {
        while (!_reject.empty()) {
          Functor<void(const std::exception &)> callback(_reject.front());
          callback(exception);
          _reject.pop();
        }

        Finally();
      }
  };

  template <> class Promise<void> : public Reference {
      friend class Deferred<void>;
      friend class Let<Promise<void> >;

    public:
      template<typename T> inline Promise* Then(T&& callback) {
        _resolve.push(Functor<void()>(callback));
        return this;
      }

      template<typename T> inline Promise* Catch(T&& callback) {
        _reject.push(Functor<void(const std::exception &)>(callback));
        return this;
      }

      template<typename T> inline Promise* Finally(T&& callback) {
        _finally.push(Functor<void()>(callback));
        return this;
      }

      template<typename T> inline static Let<Promise<void> > New(T&& callback) {
        return Let<Promise<void> >::New(std::forward<T>(callback));
      }

    private:
      template<typename T> explicit Promise(T&& callback) {
        Async::Call([this, callback]() {
          Let<Deferred<void> > Q = Let<Deferred<void> >::New(this);
          callback(Q);
        });
      }

      ~Promise() override {
      
      }

      inline void Finally() {
        while (!_finally.empty()) {
          Functor<void()> callback(_finally.front());
          callback();
          _finally.pop();
        }
      }

      std::queue<Functor<void()> > _resolve;
      std::queue<Functor<void(const std::exception &)> > _reject;
      std::queue<Functor<void()> > _finally;

    protected:
      inline void Resolve() {
        while (!_resolve.empty()) {
          Functor<void()> callback(_resolve.front());
          callback();
          _resolve.pop();
        }

        Finally();
      }

      inline void Reject(const std::exception &exception) {
        while (!_reject.empty()) {
          Functor<void(const std::exception &)> callback(_reject.front());
          callback(exception);
          _reject.pop();
        }

        Finally();
      }
  };

  template <typename R> class Deferred : public Reference {
      friend class Let<Deferred>;

    public:
      inline void Resolve(R&& result) {
        _parent->Resolve(std::forward<R>(result));
      }

      inline void Reject(const std::exception &exception) {
        _parent->Reject(exception);
      }

      ~Deferred() override {}

    private:
      explicit Deferred(Promise<R> *parent) : _parent(parent) { }
      
    protected:
      Let<Promise<R> > _parent;
  };

  template <> class Deferred<void> : public Reference {
      friend class Let<Deferred>;

    public:
      inline void Resolve() {
        _parent->Resolve();
      }

      inline void Reject(const std::exception &exception) {
        _parent->Reject(exception);
      }

      ~Deferred() override {}

    private:
      explicit Deferred(Promise<void> *parent) : _parent(parent) { }
      
    protected:
      Let<Promise<void> > _parent;
  };

  class CRTC_EXPORT RTCDataChannel : public Reference {
    
  };

  enum SignalingState {
    kStable,
    kHaveLocalOffer,
    kHaveLocalPrAnswer,
    kHaveRemoteOffer,
    kHaveRemotePrAnswer,
    kClosed,
  };

  enum IceGatheringState {
    kIceGatheringNew,
    kIceGatheringGathering,
    kIceGatheringComplete
  };

  enum IceConnectionState {
    kIceConnectionNew,
    kIceConnectionChecking,
    kIceConnectionConnected,
    kIceConnectionCompleted,
    kIceConnectionFailed,
    kIceConnectionDisconnected,
    kIceConnectionClosed,
    kIceConnectionMax,
  };
  
  enum IceTransportsType {
    kTransporTypeNone,
    kTransporTypeRelay,
    kTransporTypeNoHost,
    kTransporTypeAll
  };

  enum BundlePolicy {
    kBundlePolicyBalanced,
    kBundlePolicyMaxBundle,
    kBundlePolicyMaxCompat
  };

  enum RtcpMuxPolicy {
    kRtcpMuxPolicyNegotiate,
    kRtcpMuxPolicyRequire,
  };

  enum TcpCandidatePolicy {
    kTcpCandidatePolicyEnabled,
    kTcpCandidatePolicyDisabled
  };

  enum CandidateNetworkPolicy {
    kCandidateNetworkPolicyAll,
    kCandidateNetworkPolicyLowCost
  };

  enum ContinualGatheringPolicy {
    GATHER_ONCE,
    GATHER_CONTINUALLY
  };

  enum class RTCConfigurationType {
    kSafe,
    kAggressive
  };

  class IceServer {
    public: 
      IceServer();
      virtual ~IceServer();

      std::string uri;
      std::vector<std::string> urls;
      std::string username;
      std::string password;
  };

  typedef std::vector<IceServer> IceServers;

  class RTCConfiguration {

  };

  class CRTC_EXPORT RTCPeerConnection : public Reference {
    public:
      Let<RTCPeerConnection> New(const RTCConfiguration &config);
  };
};

#endif