
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

/**
\mainpage WebRTC C++ library
WebRTC (Web Real-Time Communication) is a collection of communications protocols and application programming interfaces that enable real-time communication over peer-to-peer connections. 
This allows web browsers to not only request resources from backend servers, but also real-time information from browsers of other users.

This enables applications such as video conferencing, file transfer, chat, or desktop sharing without the need of either internal or external plugins.

WebRTC is being standardized by the World Wide Web Consortium (W3C) and the Internet Engineering Task Force (IETF). 
The reference implementation is released as free software under the terms of a BSD license. 

WebRTC uses Real-Time Protocol to transfer audio and video.
\sa https://webrtc.org/
*/

/** @file */

#ifndef INCLUDE_CRTC_H_
#define INCLUDE_CRTC_H_

#include <utility>
#include <memory>
#include <vector>
#include <string>

#ifdef CRTC_OS_WIN
  #define CRTC_EXPORT __declspec(dllexport)
  #define CRTC_NO_EXPORT __declspec(dllimport)
#else
  #define CRTC_EXPORT __attribute__((visibility("default")))
  #define CRTC_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#ifndef CRTC_STATIC
#define CRTC_STATIC(className)                      \
  explicit className() = delete;                    \
  className(const className&) = delete;             \
  className& operator=(const className&) = delete;
#endif

#ifndef CRTC_PRIVATE
#define CRTC_PRIVATE(className)                     \
  className(const className&) = delete;             \
  className& operator=(const className&) = delete;
#endif

namespace crtc {

class CRTC_EXPORT Atomic {
    CRTC_STATIC(Atomic);

  public:
    static int Increment(volatile int *arg);
    static int Decrement(volatile int *arg);
    static int AcquireLoad(volatile int *arg);
};

class CRTC_EXPORT Time {
    CRTC_STATIC(Time);
  public:
    static int64_t Now(); // returns current time in milliseconds
    static int64_t Diff(int64_t begin, int64_t end = Now()); // returns milliseconds
    static double Since(int64_t begin, int64_t end = Now()); // returns seconds
};

/// \sa https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Statements/let

template <class T> class Let {
  public:
    template <typename... Args> inline static Let<T> New(Args&&... args) {
      return Let<T>(new Constructor<Args...>(std::forward<Args>(args)...));
    }

    inline static Let<T> Empty() {
      return Let<T>();
    }

    inline explicit Let() : _ptr(nullptr) { }

    inline ~Let() {
      Let::RemoveRef();
    }

    inline Let(T* ptr) : _ptr(ptr) {
      Let::AddRef();
    }

    inline Let(const Let<T> &src) : _ptr(*src) {
      Let::AddRef();
    }

    template <class S> inline Let(Let<S> src) : _ptr(reinterpret_cast<T*>(*src)) {
      Let::AddRef();
    }

    inline Let<T> &operator=(T* src) {
      if (src) { src->AddRef(); }

      Let::RemoveRef();

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

    inline void Dispose() const {
      if (_ptr) {
        T* ptr = _ptr;
        _ptr = nullptr;

        ptr->RemoveRef();
      }      
    }

    template <class R> inline R& operator[](const size_t index) {
      return _ptr[index];
    }

    template <class R> inline const R& operator[](const size_t index) const {
      return _ptr[index];
    }

  private:
    template <typename... Args> class Constructor : public T {
      public:
        explicit Constructor(Args&&... args) : T(std::forward<Args>(args)...), reference_count(0) { }

      protected:
        inline virtual int AddRef() const {
          return Atomic::Increment(&reference_count);
        }

        inline virtual int RemoveRef() const {
          int res = Atomic::Decrement(&reference_count);

          if (!res) {
            delete this;
          }

          return res;
        }

        inline virtual int RefCount() const {
          return Atomic::AcquireLoad(&reference_count);
        }

        virtual ~Constructor() { }

      private:
        mutable volatile int reference_count = 0;
    };

    inline void AddRef() const {
      if (_ptr) { _ptr->AddRef(); }
    }

    inline void RemoveRef() const {
      if (_ptr) { _ptr->RemoveRef(); }
    }

  protected:
    mutable T* _ptr;
};  

class CRTC_EXPORT Reference {
    template <class T> friend class Let;

  protected:
    virtual ~Reference() { }

    virtual int AddRef() const = 0;
    virtual int RemoveRef() const = 0;
    virtual int RefCount() const = 0;
};

/*
  * class Example {
  *   public:
  *     static int Foo(const char *msg) {
  *       printf("%s\n", msg);
  *       return 1337;
  *     }
  *
  *     int Bar(const char *msg) {
  *       printf("%s\n", msg);
  *       return 1337;
  *     }
  * };
  *   
  * Example ex;
  *
  * Functor<int(const char *msg)> ex1(&Example::Foo);
  * Functor<int(const char *msg)> ex2 = Functor<int(const char *msg)>(&ex, &Example::Bar);
  * Functor<int(const char *msg)> ex3 = [](const char *msg) {
  *   printf("%s\n", msg);
  *   return 1337;
  * };
  *
  * int res1 = ex1("Hello World1!");
  * int res2 = ex2("Hello World2!");
  * int res3 = ex3("Hello World3!");
  *
  * printf("Ex1: %s\n", (res1 == 1337) ? "OK" : "FAILED");
  * printf("Ex2: %s\n", (res2 == 1337) ? "OK" : "FAILED"); 
  * printf("Ex3: %s\n", (res3 == 1337) ? "OK" : "FAILED"); 
  *
  */

template <typename T> class CRTC_EXPORT Functor;
template <typename R, typename... Args> class CRTC_EXPORT Functor<R(Args...)> {
  public:
    enum Flags {
      kNone = 1 << 1,
      kOnce = 1 << 2,
    };

    explicit Functor() : _flags(kNone) { }
    virtual ~Functor() { }

    Functor(const Functor<R(Args...)> &functor, const Functor<void()> &notifier) : _flags(kOnce), _callback(Let<VerifyWrap<void>>::New(functor, notifier)) { }
    Functor(const Functor<R(Args...)> &functor) : _flags(functor._flags), _callback(functor._callback) { }
    Functor(Functor&& functor) : _flags(functor._flags), _callback(std::move(functor._callback)) { }

    template <class T> inline Functor(const T& functor, Flags flags = kNone) : _flags(flags), _callback(Let<Wrap<T> >::New(functor)) { }
    template <class Object, class Method> inline Functor(Object *object, const Method& method, Flags flags = kNone) : _flags(flags), _callback(Let<ObjectWrap<Object, Method> >::New(object, method)) { }
    template <class Object, class Method> inline Functor(const Let<Object> &object, const Method& method, Flags flags = kNone) : _flags(flags), _callback(Let<LetWrap<Object, Method> >::New(object, method)) { }

    inline R operator()(Args... args) const {
      if (!_callback.IsEmpty()) {
        Let<Callback> callback = _callback;

        if (_flags & kOnce) {
          _callback.Dispose();
        }

        return callback->Call(std::move(args)...);
      }

      return R();
    }

    inline Functor<R(Args...)>& operator=(const Functor<R(Args...)> &functor) {
      _flags = functor._flags;
      _callback = std::move(functor._callback);
      return *this;
    }

    inline bool IsEmpty() const {
      return _callback.IsEmpty();
    }

    operator bool() const {
      return !_callback.IsEmpty();
    }

    inline void Dispose() const {
      _callback.Dispose();
    }

  private:
    class Callback : virtual public Reference {
        CRTC_PRIVATE(Callback);
        friend class Let<Callback>;

      public:
        virtual R Call(Args&&... args) const = 0;

      protected:
        explicit Callback() { }
        ~Callback() override { }
    };

    template <class T> class Wrap : public Callback {
        CRTC_PRIVATE(Wrap);
        friend class Let<Wrap>;

      public:
        inline R Call(Args&&... args) const override {
          return _functor(std::forward<Args>(args)...);
        }

      protected:
        explicit Wrap(const T& functor) : _functor(functor) { }
        ~Wrap() override { }

        T _functor;
    };

    template <class Object, class Method> class ObjectWrap : public Callback {
        CRTC_PRIVATE(ObjectWrap);
        friend class Let<ObjectWrap>;

      public:
        inline R Call(Args&&... args) const override {
          if (_object) {
            return (_object->*_method)(std::forward<Args>(args)...);
          }

          return R();
        }

      protected:
        explicit ObjectWrap(Object *object, const Method &method) : _object(object), _method(method) { }
        ~ObjectWrap() override { }

        Object* _object;
        Method _method;
    };

    template <class Object, class Method> class LetWrap : public Callback {
        CRTC_PRIVATE(LetWrap);
        friend class Let<LetWrap>;

      public:
        inline R Call(Args&&... args) const override {
          if (!_object.IsEmpty()) {
            return (_object->*_method)(std::forward<Args>(args)...);
          }
          
          return R();
        }

      protected:
        explicit LetWrap(const Let<Object> &object, const Method &method) : _object(object), _method(method) { }
        ~LetWrap() override { }

        Let<Object> _object;
        Method _method;
    };

    template <class Type = void> class VerifyWrap : public Callback {
        CRTC_PRIVATE(VerifyWrap);
        friend class Let<VerifyWrap>;

      public:
        inline R Call(Args&&... args) const override {
          _notifier.Dispose();
          return _functor(std::forward<Args>(args)...);
        }

      protected:
        explicit VerifyWrap(const Functor<R(Args...)> &functor, const Functor<Type()> &notifier) : _functor(functor), _notifier(notifier) { }

        ~VerifyWrap() override {
          _notifier();
        }

        Functor<R(Args...)> _functor;
        Functor<Type()> _notifier;
    };

    Flags _flags;
    Let<Callback> _callback;
};

typedef Functor<void()> Callback;

class CRTC_EXPORT Async {
    CRTC_STATIC(Async);
  public:
    static void Call(Callback callback, int delay = 0);
};

/// \sa https://developer.mozilla.org/en/docs/Web/API/Window/SetImmediate

template <typename F, typename... Args> static inline void SetImmediate(F&& func, Args... args) {
  Functor<void(Args...)> callback(func);

  Async::Call(Callback([=]() {
    callback(args...);
  }, [=]() {
    callback(args...);
  }));
}

/// \sa https://developer.mozilla.org/en-US/docs/Web/API/WindowTimers/setTimeout

template <typename F, typename... Args> static inline void SetTimeout(F&& func, int delay, Args... args) {
  Functor<void(Args... args)> callback(func);

  Async::Call(Callback([=]() {
    callback(args...);
  }, [=]() {
    callback(args...);
  }), (delay > 0) ? delay : 0);
}

/// \sa https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Error

class CRTC_EXPORT Error : virtual public Reference {
  public:
    static Let<Error> New(std::string message, std::string fileName = __FILE__, int lineNumber = __LINE__);

    virtual std::string Message() const = 0;
    virtual std::string FileName() const = 0;
    virtual int LineNumber() const = 0;

    virtual std::string ToString() const = 0;

  protected:
    explicit Error() { }
    ~Error() override { }
};

typedef Functor<void(const Let<Error> &error)> ErrorCallback;

/// \sa https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Promise

template <typename... Args> class Promise : virtual public Reference {
    CRTC_PRIVATE(Promise<Args...>);
    friend class Let<Promise<Args...>>;

  public:
    typedef Functor<void(Args...)> FullFilledCallback;
    typedef Callback FinallyCallback;
    typedef ErrorCallback RejectedCallback;
    typedef Functor<void(const FullFilledCallback &resolve, const RejectedCallback &reject)> ExecutorCallback;

    inline static Let<Promise<Args...>> New(const ExecutorCallback &executor) {
      Let<Promise<Args...>> self = Let<Promise<Args...>>::New();

      RejectedCallback reject([=](const Let<Error> &error) {
        if (!self.IsEmpty()) {
          for (const auto &callback: self->_onreject) {
            callback(error);
          }

          for (const auto &callback: self->_onfinally) {
            callback();
          }

          self->_onfinally.clear();
          self->_onreject.clear();
          self->_onresolve.clear();

          self.Dispose();
        }
      });

      RejectedCallback asyncReject([=](const Let<Error> &error) {
        Async::Call(Callback([=]() {
          reject(error);
        }, [=]() {
          reject(error);
        }), 0);
      });

      FullFilledCallback resolve([=](Args... args) {
        Async::Call(Callback([=]() {
          if (!self.IsEmpty()) {
            for (const auto &callback: self->_onresolve) {
              callback(std::move(args)...);
            }

            for (const auto &callback: self->_onfinally) {
              callback();
            }

            self->_onfinally.clear();
            self->_onreject.clear();
            self->_onresolve.clear();

            self.Dispose();
          }
        }, [=]() {
          asyncReject(Error::New("Reference Lost.", __FILE__, __LINE__));
        }), 0);
      }, [=]() {
        asyncReject(Error::New("Reference Lost.", __FILE__, __LINE__));
      });

      if (!executor.IsEmpty()) {
        executor(resolve, asyncReject);
      } else {
        asyncReject(Error::New("Invalid Executor Callback.", __FILE__, __LINE__));
      }

      return self;
    }

    inline Let<Promise<Args...>> Then(const FullFilledCallback &callback) {
      _onresolve.push_back(callback);
      return this;
    }

    inline Let<Promise<Args...>> Catch(const RejectedCallback &callback) {
      _onreject.push_back(callback);
      return this;
    }

    inline Let<Promise<Args...>> Finally(const FinallyCallback &callback) {
      _onfinally.push_back(callback);
      return this;
    }

  private:
    std::vector<FullFilledCallback> _onresolve;
    std::vector<RejectedCallback> _onreject;
    std::vector<FinallyCallback> _onfinally;

  protected:
    explicit Promise() { }
    ~Promise() override { }
};

template<> class Promise<void> : public Promise<> { };

/// \sa https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer

class CRTC_EXPORT ArrayBuffer : virtual public Reference {
    CRTC_PRIVATE(ArrayBuffer);

  public:
    static Let<ArrayBuffer> New(size_t byteLength = 0);
    static Let<ArrayBuffer> New(const std::string &data);
    static Let<ArrayBuffer> New(const uint8_t *data, size_t byteLength = 0);

    virtual size_t ByteLength() const = 0;

    virtual Let<ArrayBuffer> Slice(size_t begin = 0, size_t end = 0) const = 0;

    virtual uint8_t *Data() = 0;
    virtual const uint8_t *Data() const = 0;

    virtual std::string ToString() const = 0;

  protected:
    explicit ArrayBuffer() { }
    ~ArrayBuffer() override { }
};

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray

template <typename T> class CRTC_EXPORT TypedArray {
  public:
    explicit TypedArray(size_t length = 0) : 
      TypedArray(ArrayBuffer::New(length * sizeof(T))) 
    { }

    TypedArray(const T *data, size_t length = 0) :
      TypedArray(ArrayBuffer::New(data, length * sizeof(T)))
    { }

    TypedArray(const TypedArray<T> &typedArray) :
      _empty(0),
      _data(typedArray._data),
      _length(typedArray._length),
      _byteOffset(typedArray._byteOffset),
      _byteLength(typedArray._byteLength), 
      _buffer(typedArray._buffer)
    { }

    template <typename N> TypedArray(const TypedArray<N> &typedArray) :
      TypedArray(typedArray.Buffer())
    { }

    TypedArray(const Let<ArrayBuffer> &buffer, size_t byteOffset = 0, size_t byteLength = 0) :
      _empty(0),
      _data(nullptr),
      _length(0),
      _byteOffset(0),
      _byteLength(0), 
      _buffer(buffer) 
    {
      if (_buffer.IsEmpty()) {
        _buffer = ArrayBuffer::New(byteLength - byteOffset);
      }

      byteLength = (!byteLength) ? _buffer->ByteLength() : byteLength;
      byteLength -= byteOffset;

      if (byteLength && (byteLength % sizeof(T)) == 0) {
        _data = reinterpret_cast<T*>(buffer->Data() + byteOffset);
        _byteOffset = byteOffset;
        _byteLength = byteLength;
        _length = byteLength / sizeof(T);
      }
    }

    inline size_t Length() const {
      return _length;
    }

    inline size_t ByteOffset() const {
      return _byteOffset;
    }

    inline size_t ByteLength() const {
      return _byteLength;
    }

    inline Let<ArrayBuffer> Buffer() const {
      return _buffer;
    }

    inline Let<ArrayBuffer> Slice(size_t begin = 0, size_t end = 0) const {
      if (_length) {
        return _buffer->Slice(begin * sizeof(T), end * sizeof(T));
      }

      return ArrayBuffer::New();
    }

    inline T *Data() {
      return (_length) ? _data : nullptr;
    }

    inline const T *Data() const {
      return (_length) ? _data : nullptr;
    }

    inline T& Get(const size_t index) {
      if (index < _length) {
        return _data[index];
      }

      return _empty;
    }

    inline const T& Get(const size_t index) const {
      if (index < _length) {
        return _data[index];
      }

      return TypedArray<T>::empty;
    }

    inline void Set(const size_t index, const T &value) {
      if (index < _length) {
        _data[index] = index;
      }
    }

    inline T& operator[](const size_t index) {
      if (index < _length) {
        return _data[index];
      }

      return _empty;
    }

    inline const T& operator[](const size_t index) const { 
      if (index < _length) {
        return _data[index];
      }

      return TypedArray<T>::empty;
    }

  protected:
    static const T empty = 0;
    
    T _empty;
    T* _data;
    size_t _length;
    size_t _byteOffset;
    size_t _byteLength;
    Let<ArrayBuffer> _buffer;
};

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Int8Array

typedef TypedArray<int8_t> Int8Array;

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array 

typedef TypedArray<uint8_t> Uint8Array;

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Int16Array 

typedef TypedArray<int16_t> Int16Array;

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint16Array 

typedef TypedArray<uint16_t> Uint16Array;

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Int32Array 

typedef TypedArray<int32_t> Int32Array;

/// \sa https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint32Array 

typedef TypedArray<uint32_t> Uint32Array;

class CRTC_EXPORT Module {
    CRTC_STATIC(Module);

  public:
    static void Init();
    static bool DispatchEvents(bool kForever = false);
    static void Dispose();

    static void RegisterAsyncCallback(const Callback &callback);
    static void UnregisterAsyncCallback();
};

/// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStreamTrack

class MediaStreamTrack : virtual public Reference {
    CRTC_PRIVATE(MediaStreamTrack);

  public:
    enum Type {
      kAudio,
      kVideo,
    };

    enum State {
      kLive,
      kEnded,
    };

    virtual bool Enabled() const = 0;
    virtual bool Muted() const = 0;
    virtual bool Remote() const = 0;
    virtual std::string Id() const = 0;

    virtual Type Kind() const = 0;
    virtual State ReadyState() const = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStreamTrack/clone

    virtual Let<MediaStreamTrack> Clone() = 0;

    Callback onstarted;
    Callback onended;
    Callback onmute;
    Callback onunmute;

  protected:
    explicit MediaStreamTrack();
    ~MediaStreamTrack() override;
};

typedef std::vector<Let<MediaStreamTrack>> MediaStreamTracks;

/// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStream

class MediaStream : virtual public Reference {
    CRTC_PRIVATE(MediaStream);

  public:
    virtual std::string Id() const = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStream/addTrack

    virtual void AddTrack(const Let<MediaStreamTrack> &track) = 0;
    virtual void RemoveTrack(const Let<MediaStreamTrack> &track) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStream/getTrackById

    virtual Let<MediaStreamTrack> GetTrackById(const std::string &id) const = 0;

    virtual MediaStreamTracks GetAudioTracks() const = 0;
    virtual MediaStreamTracks GetVideoTracks() const = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/MediaStream/clone

    virtual Let<MediaStream> Clone() = 0;

    Functor<void(const Let<MediaStreamTrack> &track)> onaddtrack;
    Functor<void(const Let<MediaStreamTrack> &track)> onremovetrack;
    
  protected:
    explicit MediaStream();
    ~MediaStream() override;
};

typedef std::vector<Let<MediaStream>> MediaStreams;

/// \sa https://developer.mozilla.org/en/docs/Web/API/RTCDataChannel

class CRTC_EXPORT RTCDataChannel : virtual public Reference {
    CRTC_PRIVATE(RTCDataChannel);

  public:
    typedef Functor<void(const Let<ArrayBuffer> &buffer, bool binary)> MessageCallback;

    enum State {
      kConnecting,
      kOpen,
      kClosing,
      kClosed
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/id

    virtual int Id() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/label

    virtual std::string Label() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/bufferedAmount

    virtual uint64_t BufferedAmount() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/bufferedAmountLowThreshold

    virtual uint64_t BufferedAmountLowThreshold() = 0;
    virtual void SetBufferedAmountLowThreshold(uint64_t threshold = 0) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/maxPacketLifeTime
    
    virtual uint16_t MaxPacketLifeTime() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/maxRetransmits

    virtual uint16_t MaxRetransmits() = 0;
     
    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/negotiated

    virtual bool Negotiated() = 0;
     
    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/ordered

    virtual bool Ordered() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/protocol

    virtual std::string Protocol() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/readyState

    virtual State ReadyState() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/close

    virtual void Close() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/send

    virtual void Send(const Let<ArrayBuffer> &data, bool binary = true) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/onbufferedamountlow

    Callback onbufferedamountlow;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/onclose

    Callback onclose;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/onerror

    ErrorCallback onerror;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/onmessage

    MessageCallback onmessage;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCDataChannel/onopen

    Callback onopen;

  protected:
    explicit RTCDataChannel();
    ~RTCDataChannel() override;
};

/// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection

class CRTC_EXPORT RTCPeerConnection : virtual public Reference {
    CRTC_PRIVATE(RTCPeerConnection);

  public:
    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/createDataChannel#RTCDataChannelInit_dictionary
    
    typedef struct RTCDataChannelInit {
      RTCDataChannelInit() :
        id(-1),
        maxPacketLifeTime(-1),
        maxRetransmits(-1),
        ordered(true),
        negotiated(false)
      { }

      int id;
      int maxPacketLifeTime;
      int maxRetransmits;
      bool ordered;
      bool negotiated;
      std::string protocol;
    } RTCDataChannelInit;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCSessionDescription

    typedef struct {
      enum RTCSdpType {
        kAnswer,
        kOffer,
        kPranswer,
        kRollback,
      };

      RTCSdpType type;
      std::string sdp;
    } RTCSessionDescription;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/signalingState

    enum RTCSignalingState {
      kStable,
      kHaveLocalOffer,
      kHaveLocalPrAnswer,
      kHaveRemoteOffer,
      kHaveRemotePrAnswer,
      kSignalingClosed,
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/iceGatheringState

    enum RTCIceGatheringState {
      kNewGathering,
      kGathering,
      kComplete
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/iceConnectionState

    enum RTCIceConnectionState {
      kNew,
      kChecking,
      kConnected,
      kCompleted,
      kFailed,
      kDisconnected,
      kClosed,
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCConfiguration#RTCBundlePolicy_enum

    enum RTCBundlePolicy {
      kBalanced,
      kMaxBundle,
      kMaxCompat
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCConfiguration#RTCIceTransportPolicy_enum

    enum RTCIceTransportPolicy {
      kRelay,
      kPublic,
      kAll
    };

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCConfiguration#RTCRtcpMuxPolicy_enum

    enum RTCRtcpMuxPolicy {
      kNegotiate,
      kRequire,
    };

    // \sa https://developer.mozilla.org/en/docs/Web/API/RTCIceCandidate

    typedef struct {
      std::string candidate;
      std::string sdpMid;
      uint32_t sdpMLineIndex;
    } RTCIceCandidate;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCIceServer

    typedef struct {
      std::string credential;
      std::string credentialType;
      std::string username;
      std::vector<std::string> urls;
    } RTCIceServer;

    typedef std::vector<RTCIceServer> RTCIceServers;

    static RTCIceServers defaultIceServers;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCConfiguration

    typedef struct RTCConfiguration {
      explicit RTCConfiguration();
      ~RTCConfiguration();

      uint16_t iceCandidatePoolSize;
      RTCBundlePolicy bundlePolicy;       
      RTCIceServers iceServers;
      RTCIceTransportPolicy iceTransportPolicy;
      RTCRtcpMuxPolicy rtcpMuxPolicy;
    } RTCConfiguration;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/createOffer#RTCOfferOptions_dictionary
    /// \sa https://w3c.github.io/webrtc-pc/#idl-def-rtcofferansweroptions

    typedef struct {
      bool voiceActivityDetection;
    } RTCOfferAnswerOptions;

    /// \sa https://w3c.github.io/webrtc-pc/#idl-def-rtcofferoptions

    typedef struct : RTCOfferAnswerOptions {
      bool iceRestart;
    } RTCOfferOptions;

    /// \sa https://w3c.github.io/webrtc-pc/#idl-def-rtcansweroptions

    typedef struct : RTCOfferAnswerOptions {
    } RTCAnswerOptions;

    typedef Functor<void(const Let<MediaStream> &stream)> StreamCallback;
    typedef Functor<void(const Let<RTCDataChannel> &dataChannel)> DataChannelCallback;
    typedef Functor<void(const RTCIceCandidate &candidate)> IceCandidateCallback;

    static Let<RTCPeerConnection> New(const RTCConfiguration &config = RTCConfiguration());

    virtual Let<RTCDataChannel> CreateDataChannel(const std::string &label, const RTCDataChannelInit &options = RTCDataChannelInit()) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/addIceCandidate

    virtual Let<Promise<void>> AddIceCandidate(const RTCIceCandidate &candidate) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/addStream

    virtual void AddStream(const Let<MediaStream> &stream) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/createAnswer

    virtual Let<Promise<RTCSessionDescription>> CreateAnswer(const RTCAnswerOptions &options = RTCAnswerOptions()) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/createOffer

    virtual Let<Promise<RTCSessionDescription>> CreateOffer(const RTCOfferOptions &options = RTCOfferOptions()) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/getLocalStreams

    virtual MediaStreams GetLocalStreams() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/getRemoteStreams

    virtual MediaStreams GetRemoteStreams() = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/removeStream

    virtual void RemoveStream(const Let<MediaStream> &stream) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/setConfiguration

    virtual void SetConfiguration(const RTCConfiguration &config) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/setLocalDescription

    virtual Let<Promise<void>> SetLocalDescription(const RTCSessionDescription &sdp) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/setRemoteDescription

    virtual Let<Promise<void>> SetRemoteDescription(const RTCSessionDescription &sdp) = 0;

    /// \sa https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/close

    virtual void Close() = 0;

    virtual RTCSessionDescription CurrentLocalDescription() = 0;
    virtual RTCSessionDescription CurrentRemoteDescription() = 0;
    virtual RTCSessionDescription LocalDescription() = 0;
    virtual RTCSessionDescription PendingLocalDescription() = 0;
    virtual RTCSessionDescription PendingRemoteDescription() = 0;
    virtual RTCSessionDescription RemoteDescription() = 0; 

    virtual RTCIceConnectionState IceConnectionState() = 0;
    virtual RTCIceGatheringState IceGatheringState() = 0;
    virtual RTCSignalingState SignalingState() = 0;

    Callback onnegotiationneeded;
    Callback onsignalingstatechange;
    Callback onicegatheringstatechange;
    Callback oniceconnectionstatechange;
    Callback onicecandidatesremoved;
    StreamCallback onaddstream;
    StreamCallback onremovestream;
    DataChannelCallback ondatachannel;
    IceCandidateCallback onicecandidate;

  protected:
    explicit RTCPeerConnection();
    ~RTCPeerConnection() override;
};
}; // namespace crtc

#endif // INCLUDE_CRTC_H_