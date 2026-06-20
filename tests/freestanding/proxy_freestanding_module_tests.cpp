// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#if __STDC_HOSTED__
#error "This file shall be compiled targeting a freestanding environment."
#endif // __STDC_HOSTED__

#include <proxy/proxy_macros.h>

#include <new>
#include <type_traits>
#include <utility>

import proxy.v4;

constexpr unsigned DefaultHash = -1;
unsigned GetHashImpl(int v) { return static_cast<unsigned>(v + 3) * 31; }
unsigned GetHashImpl(double v) { return static_cast<unsigned>(v * v + 5) * 87; }
unsigned GetHashImpl(const char* v) {
  unsigned result = 91u;
  for (int i = 0; v[i]; ++i) {
    result = result * 47u + v[i];
  }
  return result;
}
unsigned GetHashImpl(auto&&) { return DefaultHash; }
PRO_DEF_FREE_DISPATCH(FreeGetHash, GetHashImpl, GetHash);

struct Hashable : pro::facade_builder                       //
                  ::add_convention<FreeGetHash, unsigned()> //
                  ::build {};

extern "C" int main() {
  int i = 123;
  double d = 3.14159;
  const char* s = "lalala";
  pro::proxy<Hashable> p;
  p = &i;
  if (GetHash(*p) != GetHashImpl(i)) {
    return 1;
  }
  p = &d;
  if (GetHash(*p) != GetHashImpl(d)) {
    return 1;
  }
  p = pro::make_proxy_inplace<Hashable>(s);
  if (GetHash(*p) != GetHashImpl(s)) {
    return 1;
  }
  return 0;
}
