/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* A class for optional values and in-place lazy construction. */

#ifndef mozilla_Maybe_h
#define mozilla_Maybe_h

#include "mozilla/Alignment.h"
#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/Move.h"
#include "mozilla/OperatorNewExtensions.h"
#include "mozilla/TypeTraits.h"

#include <new>  // for placement new
#include <ostream>
#include <type_traits>

namespace mozilla {

struct Nothing { };

template<class T>
class MOZ_NON_PARAM MOZ_INHERIT_TYPE_ANNOTATIONS_FROM_TEMPLATE_ARGS Maybe
{
  MOZ_ALIGNAS_IN_STRUCT(T) unsigned char mStorage[sizeof(T)];
  char mIsSome; // not bool -- guarantees minimal space consumption

  // GCC fails due to -Werror=strict-aliasing if |mStorage| is directly cast to
  // T*.  Indirecting through these functions addresses the problem.
  void* data() { return mStorage; }
  const void* data() const { return mStorage; }

public:
  using ValueType = T;

  Maybe() : mIsSome(false) { }
  ~Maybe() { reset(); }

  MOZ_IMPLICIT Maybe(Nothing) : mIsSome(false) { }

  Maybe(const Maybe& aOther)
    : mIsSome(false)
  {
    if (aOther.mIsSome) {
      emplace(*aOther);
    }
  }

  /**
   * Maybe<T> can be copy-constructed from a Maybe<U> if U is convertible to T.
   */
  template<typename U,
           typename =
             typename std::enable_if<std::is_convertible<U, T>::value>::type>
  MOZ_IMPLICIT
  Maybe(const Maybe<U>& aOther)
    : mIsSome(false)
  {
    if (aOther.isSome()) {
      emplace(*aOther);
    }
  }

  Maybe(Maybe&& aOther)
    : mIsSome(false)
  {
    if (aOther.mIsSome) {
      emplace(Move(*aOther));
      aOther.reset();
    }
  }

  /**
   * Maybe<T> can be move-constructed from a Maybe<U> if U is convertible to T.
   */
  template<typename U,
           typename =
             typename std::enable_if<std::is_convertible<U, T>::value>::type>
  MOZ_IMPLICIT
  Maybe(Maybe<U>&& aOther)
    : mIsSome(false)
  {
    if (aOther.isSome()) {
      emplace(Move(*aOther));
      aOther.reset();
    }
  }

  Maybe& operator=(const Maybe& aOther)
  {
    if (&aOther != this) {
      if (aOther.mIsSome) {
        if (mIsSome) {
          ref() = aOther.ref();
        } else {
          emplace(*aOther);
        }
      } else {
        reset();
      }
    }
    return *this;
  }

  template<typename U,
           typename =
             typename std::enable_if<std::is_convertible<U, T>::value>::type>
  Maybe& operator=(const Maybe<U>& aOther)
  {
    if (aOther.isSome()) {
      if (mIsSome) {
        ref() = aOther.ref();
      } else {
        emplace(*aOther);
      }
    } else {
      reset();
    }
    return *this;
  }

  Maybe& operator=(Maybe&& aOther)
  {
    MOZ_ASSERT(this != &aOther, "Self-moves are prohibited");

    if (aOther.mIsSome) {
      if (mIsSome) {
        ref() = Move(aOther.ref());
      } else {
        emplace(Move(*aOther));
      }
      aOther.reset();
    } else {
      reset();
    }

    return *this;
  }

  template<typename U,
           typename =
             typename std::enable_if<std::is_convertible<U, T>::value>::type>
  Maybe& operator=(Maybe<U>&& aOther)
  {
    if (aOther.isSome()) {
      if (mIsSome) {
        ref() = Move(aOther.ref());
      } else {
        emplace(Move(*aOther));
      }
      aOther.reset();
    } else {
      reset();
    }

    return *this;
  }

  /* Methods that check whether this Maybe contains a value */
  explicit operator bool() const { return isSome(); }
  bool isSome() const { return mIsSome; }
  bool isNothing() const { return !mIsSome; }

  /* Returns the contents of this Maybe<T> by value. Unsafe unless |isSome()|. */
  T value() const
  {
    MOZ_ASSERT(mIsSome);
    return ref();
  }

  /*
   * Returns the contents of this Maybe<T> by value. If |isNothing()|, returns
   * the default value provided.
   */
  template<typename V>
  T valueOr(V&& aDefault) const
  {
    if (isSome()) {
      return ref();
    }
    return Forward<V>(aDefault);
  }

  /*
   * Returns the contents of this Maybe<T> by value. If |isNothing()|, returns
   * the value returned from the function or functor provided.
   */
  template<typename F>
  T valueOrFrom(F&& aFunc) const
  {
    if (isSome()) {
      return ref();
    }
    return aFunc();
  }

  /* Returns the contents of this Maybe<T> by pointer. Unsafe unless |isSome()|. */
  T* ptr()
  {
    MOZ_ASSERT(mIsSome);
    return &ref();
  }

  const T* ptr() const
  {
    MOZ_ASSERT(mIsSome);
    return &ref();
  }

  /*
   * Returns the contents of this Maybe<T> by pointer. If |isNothing()|,
   * returns the default value provided.
   */
  T* ptrOr(T* aDefault)
  {
    if (isSome()) {
      return ptr();
    }
    return aDefault;
  }

  const T* ptrOr(const T* aDefault) const
  {
    if (isSome()) {
      return ptr();
    }
    return aDefault;
  }

  /*
   * Returns the contents of this Maybe<T> by pointer. If |isNothing()|,
   * returns the value returned from the function or functor provided.
   */
  template<typename F>
  T* ptrOrFrom(F&& aFunc)
  {
    if (isSome()) {
      return ptr();
    }
    return aFunc();
  }

  template<typename F>
  const T* ptrOrFrom(F&& aFunc) const
  {
    if (isSome()) {
      return ptr();
    }
    return aFunc();
  }

  T* operator->()
  {
    MOZ_ASSERT(mIsSome);
    return ptr();
  }

  const T* operator->() const
  {
    MOZ_ASSERT(mIsSome);
    return ptr();
  }

  /* Returns the contents of this Maybe<T> by ref. Unsafe unless |isSome()|. */
  T& ref()
  {
    MOZ_ASSERT(mIsSome);
    return *static_cast<T*>(data());
  }

  const T& ref() const
  {
    MOZ_ASSERT(mIsSome);
    return *static_cast<const T*>(data());
  }

  /*
   * Returns the contents of this Maybe<T> by ref. If |isNothing()|, returns
   * the default value provided.
   */
  T& refOr(T& aDefault)
  {
    if (isSome()) {
      return ref();
    }
    return aDefault;
  }

  const T& refOr(const T& aDefault) const
  {
    if (isSome()) {
      return ref();
    }
    return aDefault;
  }

  /*
   * Returns the contents of this Maybe<T> by ref. If |isNothing()|, returns the
   * value returned from the function or functor provided.
   */
  template<typename F>
  T& refOrFrom(F&& aFunc)
  {
    if (isSome()) {
      return ref();
    }
    return aFunc();
  }

  template<typename F>
  const T& refOrFrom(F&& aFunc) const
  {
    if (isSome()) {
      return ref();
    }
    return aFunc();
  }

  T& operator*()
  {
    MOZ_ASSERT(mIsSome);
    return ref();
  }

  const T& operator*() const
  {
    MOZ_ASSERT(mIsSome);
    return ref();
  }

  /* If |isSome()|, runs the provided function or functor on the contents of
   * this Maybe. */
  template<typename Func>
  Maybe& apply(Func aFunc)
  {
    if (isSome()) {
      aFunc(ref());
    }
    return *this;
  }

  template<typename Func>
  const Maybe& apply(Func aFunc) const
  {
    if (isSome()) {
      aFunc(ref());
    }
    return *this;
  }

  /*
   * If |isSome()|, runs the provided function and returns the result wrapped
   * in a Maybe. If |isNothing()|, returns an empty Maybe value.
   */
  template<typename Func>
  auto map(Func aFunc) -> Maybe<decltype(aFunc(DeclVal<Maybe<T>>().ref()))>
  {
    using ReturnType = decltype(aFunc(ref()));
    if (isSome()) {
      Maybe<ReturnType> val;
      val.emplace(aFunc(ref()));
      return val;
    }
    return Maybe<ReturnType>();
  }

  template<typename Func>
  auto map(Func aFunc) const -> Maybe<decltype(aFunc(DeclVal<Maybe<T>>().ref()))>
  {
    using ReturnType = decltype(aFunc(ref()));
    if (isSome()) {
      Maybe<ReturnType> val;
      val.emplace(aFunc(ref()));
      return val;
    }
    return Maybe<ReturnType>();
  }

  /* If |isSome()|, empties this Maybe and destroys its contents. */
  void reset()
  {
    if (isSome()) {
      ref().T::~T();
      mIsSome = false;
    }
  }

  /*
   * Constructs a T value in-place in this empty Maybe<T>'s storage. The
   * arguments to |emplace()| are the parameters to T's constructor.
   */
  template<typename... Args>
  void emplace(Args&&... aArgs)
  {
    MOZ_ASSERT(!mIsSome);
    ::new (KnownNotNull, data()) T(Forward<Args>(aArgs)...);
    mIsSome = true;
  }

  friend std::ostream&
  operator<<(std::ostream& aStream, const Maybe<T>& aMaybe)
  {
    if (aMaybe) {
      aStream << aMaybe.ref();
    } else {
      aStream << "<Nothing>";
    }
    return aStream;
  }
};

} // namespace mozilla

#endif /* mozilla_Maybe_h */
