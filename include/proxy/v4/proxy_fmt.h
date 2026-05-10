// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_PROXY_FMT_H_
#define MSFT_PROXY_V4_PROXY_FMT_H_

#include <string_view>
#include <type_traits>

#ifndef __msft_lib_proxy4
#error Please ensure that proxy.h is included before proxy_fmt.h.
#endif // __msft_lib_proxy4

#if FMT_VERSION < 60100
#error Please ensure that the appropriate {fmt} headers (version 6.1.0 or \
later) are included before proxy_fmt.h.
#endif // FMT_VERSION < 60100

namespace pro::inline v4 {

namespace details {

template <class CharT>
#if FMT_VERSION >= 110000
using fmt_buffered_context = fmt::buffered_context<CharT>;
#else
using fmt_buffered_context = fmt::buffer_context<CharT>;
#endif // FMT_VERSION

struct fmt_format_traits
    : format_traits<fmt::formatter, std::basic_string_view,
                    fmt::basic_format_parse_context, fmt_buffered_context> {};

} // namespace details

namespace skills {

template <class FB>
using fmt_format = typename FB::template add_convention<
    details::fmt_format_traits::dispatch,
    details::fmt_format_traits::overload<char>>;

template <class FB>
using fmt_wformat = typename FB::template add_convention<
    details::fmt_format_traits::dispatch,
    details::fmt_format_traits::overload<wchar_t>>;

} // namespace skills

} // namespace pro::inline v4

namespace fmt {

template <class T, class CharT>
  requires(pro::v4::details::enabled_for<T, fmt::formatter, CharT>)
struct formatter<T, CharT>
    : pro::v4::details::fmt_format_traits::formatter<CharT> {};

} // namespace fmt

#endif // MSFT_PROXY_V4_PROXY_FMT_H_
