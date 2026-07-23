// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_SKILLS_H_
#define MSFT_PROXY_V4_DETAIL_SKILLS_H_

#include <memory>
#include <version>

#if __STDC_HOSTED__
// LLVM libc++ 17 has usable <format> despite lacking __cpp_lib_format until 19.
#if __cpp_lib_format >= 201907L || _LIBCPP_VERSION >= 170000
#include <format>
#include <string_view>
#define PRO4D_HAS_FORMAT
#endif // __cpp_lib_format || _LIBCPP_VERSION >= 170000
#endif // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
#include <optional>
#include <typeinfo>
#endif // __cpp_rtti >= 199711L

#include "core.h"

namespace pro::inline v4 {

#if __cpp_rtti >= 199711L
class bad_proxy_cast : public std::bad_cast {
public:
  char const* what() const noexcept override {
    return "pro::v4::bad_proxy_cast";
  }
};
#endif // __cpp_rtti >= 199711L

namespace detail {

template <template <class...> class TT, class... Ctx>
struct enabled_t {};
template <class T, template <class...> class TT, class... Ctx>
concept enabled_for = std::is_base_of_v<enabled_t<TT, Ctx...>, T>;

struct view_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class T>
  PRO4D_STATIC_CALL(auto, T& value) noexcept
    requires(requires {
      { std::addressof(*value) } noexcept;
    })
  {
    return observer_ptr<decltype(*value), decltype(*std::as_const(value)),
                        decltype(*std::move(value)),
                        decltype(*std::move(std::as_const(value)))>{*value};
  }
};
template <class F>
using view_conversion_overload = proxy_view<F>() & noexcept;

struct weak_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class P>
  PRO4D_STATIC_CALL(auto, const P& self) noexcept
    requires(requires(const typename P::weak_type& w) {
      { w.lock() } noexcept;
    } && std::is_convertible_v<const P&, typename P::weak_type>)
  {
    return converter{
        [&self]<class F>(std::in_place_type_t<proxy<F>>) noexcept
          requires(proxiable<typename P::weak_type, F>)
        { return proxy<F>{std::in_place_type<typename P::weak_type>, self}; }};
  }
};
template <class F>
using weak_conversion_overload = weak_proxy<F>() const noexcept;

template <template <class...> class Formatter,
          template <class...> class StringView,
          template <class...> class ParseContext,
          template <class...> class FormatContext>
struct format_traits {
  template <class CharT>
  using overload = typename FormatContext<CharT>::iterator(
      StringView<CharT> spec, FormatContext<CharT>& fc) const;

  struct dispatch {
    template <class T, class CharT>
    PRO4D_STATIC_CALL(auto, const T& self, StringView<CharT> spec,
                      FormatContext<CharT>& fc)
      requires(std::is_default_constructible_v<Formatter<T, CharT>>)
    {
      Formatter<T, CharT> impl;
      {
        ParseContext<CharT> pc{spec};
        impl.parse(pc);
      }
      return impl.format(self, fc);
    }

    template <class P, class D, class... Os>
    struct PRO4D_ENFORCE_EBO accessor : accessor<P, D, Os>... {};
    template <class P, class D>
    struct accessor<P, D, overload<char>> : enabled_t<Formatter, char> {};
    template <class P, class D>
    struct accessor<P, D, overload<wchar_t>> : enabled_t<Formatter, wchar_t> {};
  };

  template <class CharT>
  struct formatter {
    constexpr auto parse(ParseContext<CharT>& pc) {
      for (auto it = pc.begin(); it != pc.end(); ++it) {
        if (*it == '}') {
          spec_ = StringView<CharT>{pc.begin(), it + 1};
          return it;
        }
      }
      return pc.end();
    }

    template <class P, class CompatibleFormatContext>
    auto format(const P& p, CompatibleFormatContext& fc) const
        -> CompatibleFormatContext::iterator {
      return invoke<dispatch, overload<CharT>>(p, spec_, fc);
    }

  private:
    StringView<CharT> spec_;
  };
};

#ifdef PRO4D_HAS_FORMAT
template <class CharT>
struct std_format_context_traits;
template <>
struct std_format_context_traits<char>
    : std::type_identity<std::format_context> {};
template <>
struct std_format_context_traits<wchar_t>
    : std::type_identity<std::wformat_context> {};
template <class CharT>
using std_format_context = std_format_context_traits<CharT>::type;
struct std_format_traits
    : format_traits<std::formatter, std::basic_string_view,
                    std::basic_format_parse_context, std_format_context> {};
#endif // PRO4D_HAS_FORMAT

#if __cpp_rtti >= 199711L
struct proxy_cast_context {
  const std::type_info* type_ptr;
  bool is_ref;
  bool is_const;
  void* result_ptr;
};

template <class Self, class D, class O>
struct proxy_cast_accessor_impl {
  template <class T>
  friend T proxy_cast(Self self) {
    static_assert(!std::is_rvalue_reference_v<T>);
    if constexpr (std::is_lvalue_reference_v<T>) {
      using U = std::remove_reference_t<T>;
      void* result = nullptr;
      proxy_cast_context ctx{.type_ptr = &typeid(T),
                             .is_ref = true,
                             .is_const = std::is_const_v<U>,
                             .result_ptr = &result};
      invoke<D, O>(static_cast<Self>(self), ctx);
      if (result == nullptr) [[unlikely]] {
        PRO4D_THROW(bad_proxy_cast{});
      }
      return *static_cast<U*>(result);
    } else {
      std::optional<std::remove_const_t<T>> result;
      proxy_cast_context ctx{.type_ptr = &typeid(T),
                             .is_ref = false,
                             .is_const = false,
                             .result_ptr = &result};
      invoke<D, O>(static_cast<Self>(self), ctx);
      if (!result.has_value()) [[unlikely]] {
        PRO4D_THROW(bad_proxy_cast{});
      }
      return std::move(*result);
    }
  }
  template <class T>
  friend T* proxy_cast(std::remove_reference_t<Self>* self) noexcept
    requires(std::is_lvalue_reference<Self>::value)
  {
    void* result = nullptr;
    proxy_cast_context ctx{.type_ptr = &typeid(T),
                           .is_ref = true,
                           .is_const = std::is_const_v<T>,
                           .result_ptr = &result};
    invoke<D, O>(*self, ctx);
    return static_cast<T*>(result);
  }
};

#define PRO4D_DEF_PROXY_CAST_ACCESSOR(oq, pq, ne, ...)                         \
  template <class P, class D>                                                  \
  struct accessor<P, D, void(proxy_cast_context) oq ne>                        \
      : proxy_cast_accessor_impl<P pq, D, void(proxy_cast_context) oq ne> {}
struct proxy_cast_dispatch {
  template <class T>
  PRO4D_STATIC_CALL(void, T&& self, proxy_cast_context ctx) {
    if (typeid(T) == *ctx.type_ptr) [[likely]] {
      if (ctx.is_ref) {
        if constexpr (std::is_lvalue_reference_v<T>) {
          if (ctx.is_const || !std::is_const_v<T>) [[likely]] {
            *static_cast<void**>(ctx.result_ptr) = (void*)std::addressof(self);
          }
        }
      } else {
        if constexpr (std::is_constructible_v<std::decay_t<T>, T>) {
          static_cast<std::optional<std::decay_t<T>>*>(ctx.result_ptr)
              ->emplace(std::forward<T>(self));
        }
      }
    }
  }
  PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PRO4D_DEF_PROXY_CAST_ACCESSOR)
};
#undef PRO4D_DEF_PROXY_CAST_ACCESSOR

struct proxy_typeid_reflector {
  proxy_typeid_reflector() = default;
  template <class T>
  constexpr explicit proxy_typeid_reflector(std::in_place_type_t<T>)
      : info(&typeid(T)) {}

  template <class Self, class R>
  struct accessor {
    friend const std::type_info& proxy_typeid(const Self& self) noexcept {
      const proxy_typeid_reflector& refl = reflect<R>(self);
      return *refl.info;
    }
    PRO4D_DEBUG(
        accessor() noexcept { std::ignore = &pro_symbol_guard; }

        private : static inline const std::type_info& pro_symbol_guard(
            const Self& self) { return proxy_typeid(self); })
  };

  const std::type_info* info;
};
#endif // __cpp_rtti >= 199711L

} // namespace detail

namespace skills {

#ifdef PRO4D_HAS_FORMAT
template <class FB>
using format =
    FB::template add_convention<detail::std_format_traits::dispatch,
                                detail::std_format_traits::overload<char>>;

template <class FB>
using wformat =
    FB::template add_convention<detail::std_format_traits::dispatch,
                                detail::std_format_traits::overload<wchar_t>>;
#endif // PRO4D_HAS_FORMAT

#if __cpp_rtti >= 199711L
template <class FB>
using indirect_rtti = FB::template add_indirect_convention<
    detail::proxy_cast_dispatch, void(detail::proxy_cast_context) &,
    void(detail::proxy_cast_context) const&,
    void(detail::proxy_cast_context) &&>::
    template add_indirect_reflection<detail::proxy_typeid_reflector>;

template <class FB>
using direct_rtti =
    FB::template add_direct_convention<detail::proxy_cast_dispatch,
                                       void(detail::proxy_cast_context) &,
                                       void(detail::proxy_cast_context) const&,
                                       void(detail::proxy_cast_context) &&>::
        template add_direct_reflection<detail::proxy_typeid_reflector>;

template <class FB>
using rtti = indirect_rtti<FB>;
#endif // __cpp_rtti >= 199711L

template <class FB>
using slim = FB::template restrict_layout<sizeof(void*), alignof(void*)>;

template <class FB>
using as_view = FB::template add_direct_convention<
    detail::view_conversion_dispatch,
    facade_aware_overload_t<detail::view_conversion_overload>>;

template <class FB>
using as_weak = FB::template add_direct_convention<
    detail::weak_conversion_dispatch,
    facade_aware_overload_t<detail::weak_conversion_overload>>;

} // namespace skills

} // namespace pro::inline v4

#ifdef PRO4D_HAS_FORMAT
namespace std {

template <class T, class CharT>
  requires(pro::v4::detail::enabled_for<T, std::formatter, CharT>)
struct formatter<T, CharT>
    : pro::v4::detail::std_format_traits::formatter<CharT> {};

} // namespace std
#endif // PRO4D_HAS_FORMAT

#endif // MSFT_PROXY_V4_DETAIL_SKILLS_H_
