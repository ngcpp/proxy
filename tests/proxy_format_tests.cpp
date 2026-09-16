// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <proxy/proxy.h>

#ifdef PRO5D_HAS_FORMAT
namespace proxy_format_tests_detail {

struct NonFormattable : pro::facade_builder::build {};

static_assert(
    !std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<NonFormattable>, char>>);
static_assert(
    !std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<NonFormattable>, wchar_t>>);

struct Formattable : pro::facade_builder               //
                     ::add_skill<pro::skills::format>  //
                     ::add_skill<pro::skills::wformat> //
                     ::build {};

static_assert(std::is_default_constructible_v<
              std::formatter<pro::proxy_indirect_accessor<Formattable>, char>>);
static_assert(
    std::is_default_constructible_v<
        std::formatter<pro::proxy_indirect_accessor<Formattable>, wchar_t>>);

} // namespace proxy_format_tests_detail

namespace detail = proxy_format_tests_detail;
#endif // PRO5D_HAS_FORMAT

TEST(ProxyFormatTests, TestFormat) {
#ifdef PRO5D_HAS_FORMAT
  int v = 123;
  pro::proxy<detail::Formattable> p = &v;
  ASSERT_EQ(std::format("{}", *p), "123");
  ASSERT_EQ(std::format("{:*<6}", *p), "123***");
#else
  GTEST_SKIP() << "std::format not available";
#endif // PRO5D_HAS_FORMAT
}

TEST(ProxyFormatTests, TestWformat) {
#ifdef PRO5D_HAS_FORMAT
  int v = 123;
  pro::proxy<detail::Formattable> p = &v;
  ASSERT_EQ(std::format(L"{}", *p), L"123");
  ASSERT_EQ(std::format(L"{:*<6}", *p), L"123***");
#else
  GTEST_SKIP() << "std::format not available";
#endif // PRO5D_HAS_FORMAT
}
