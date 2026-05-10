// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <gtest/gtest.h>

#include <fmt/format.h> // <fmt/xchar.h> is intentionally not included in this TU
#include <proxy/proxy.h>
#include <proxy/proxy_fmt.h>

namespace proxy_fmt_format_no_wchar_tests_details {

struct NonFormattable : pro::facade_builder::build {};

static_assert(
    !std::is_default_constructible_v<
        fmt::formatter<pro::proxy_indirect_accessor<NonFormattable>, char>>);

struct Formattable : pro::facade_builder                  //
                     ::add_skill<pro::skills::fmt_format> //
                     ::build {};

static_assert(std::is_default_constructible_v<
              fmt::formatter<pro::proxy_indirect_accessor<Formattable>, char>>);

} // namespace proxy_fmt_format_no_wchar_tests_details

namespace details = proxy_fmt_format_no_wchar_tests_details;

TEST(ProxyFmtFormatNoWcharTests, TestFormat) {
  int v = 123;
  pro::proxy<details::Formattable> p = &v;
  ASSERT_EQ(fmt::format("{}", *p), "123");
  ASSERT_EQ(fmt::format("{:*<6}", *p), "123***");
}
