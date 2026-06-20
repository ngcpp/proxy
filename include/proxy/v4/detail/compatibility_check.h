// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_COMPATIBILITY_CHECK_H_
#define MSFT_PROXY_V4_DETAIL_COMPATIBILITY_CHECK_H_

#if (defined(_MSVC_LANG) ? _MSVC_LANG : __cplusplus) < 202002L
#error "Proxy requires C++20 or later."
#endif

// clang-cl miscalculates the layout of an empty [[msvc::no_unique_address]]
// member in a base class (llvm/llvm-project#143245), corrupting pro::proxy.
#if __has_cpp_attribute(msvc::no_unique_address)
namespace pro::inline v4::detail::compatibility_check {
struct empty {};
struct base {
  [[msvc::no_unique_address]] empty value;
};
struct derived : base {
  char dummy;
};
static_assert(sizeof(derived) == sizeof(char),
              "[[msvc::no_unique_address]] is broken");
} // namespace pro::inline v4::detail::compatibility_check
#endif

#endif // MSFT_PROXY_V4_DETAIL_COMPATIBILITY_CHECK_H_
