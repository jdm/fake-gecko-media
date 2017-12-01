/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* A class for optional values and in-place lazy construction. */

#ifndef mozilla_Maybe_h
#define mozilla_Maybe_h

#include "mozilla/Alignment.h"

namespace mozilla {

template<class T>
class Maybe
{
  MOZ_ALIGNAS_IN_STRUCT(T) unsigned char mStorage[sizeof(T)];
  char mIsSome; // not bool -- guarantees minimal space consumption

  // GCC fails due to -Werror=strict-aliasing if |mStorage| is directly cast to
  // T*.  Indirecting through these functions addresses the problem.
  void* data() { return mStorage; }
  const void* data() const { return mStorage; }

public:

  Maybe() : mIsSome(false) { }
  ~Maybe() { reset(); }

  bool isSome() const { return mIsSome; }

  T& ref()
  {
    return *static_cast<T*>(data());
  }

  void reset()
  {
    if (isSome()) {
      ref().T::~T();
      mIsSome = false;
    }
  }
};

} // namespace mozilla

#endif /* mozilla_Maybe_h */
