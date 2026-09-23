// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_DISPATCH_H_
#define MSFT_PROXY_V4_DETAIL_DISPATCH_H_

#include <exception>

#include "core.h"

namespace pro::inline v4 {

namespace detail {

template <std::size_t N>
struct sign {
  consteval sign(const char (&str)[N + 1]) {
    if (str[N] != '\0') {
      PRO4D_UNREACHABLE();
    }
    for (std::size_t i = 0; i < N; ++i) {
      value[i] = str[i];
    }
  }

  char value[N];
};
template <std::size_t N>
sign(const char (&str)[N]) -> sign<N - 1u>;

// When std::reference_constructs_from_temporary_v (C++23) is not available, we
// fall back to a conservative approximation that disallows binding a temporary
// to a reference type if the source type is not a reference or if the source
// and target reference types are not compatible.
template <class T, class U>
concept explicitly_convertible =
    std::is_constructible_v<U, T> &&
#if __cpp_lib_reference_from_temporary >= 202202L
    !std::reference_constructs_from_temporary_v<U, T>;
#else
    (!std::is_reference_v<U> ||
     (std::is_reference_v<T> &&
      std::is_convertible_v<std::add_pointer_t<std::remove_reference_t<T>>,
                            std::add_pointer_t<std::remove_reference_t<U>>>));
#endif // __cpp_lib_reference_from_temporary >= 202202L

struct noreturn_conversion {
  template <class T>
  [[noreturn]] PRO4D_STATIC_CALL(T, std::in_place_type_t<T>) {
    PRO4D_UNREACHABLE();
  }
};
using wildcard = converter<noreturn_conversion>;

} // namespace detail

template <detail::sign Sign, bool Rhs = false>
struct operator_dispatch;

#define PRO4D_DEF_LHS_LEFT_OP_ACCESSOR(oq, pq, ne, ...)                        \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    R __VA_ARGS__() oq ne {                                                    \
      return invoke<D, R() oq ne>(static_cast<P pq>(*this));                   \
    }                                                                          \
  }
#define PRO4D_DEF_LHS_UNARY_OP_ACCESSOR(oq, pq, ne, ...)                       \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    decltype(auto) __VA_ARGS__() oq ne {                                       \
      invoke<D, R() oq ne>(static_cast<P pq>(*this));                          \
      return static_cast<P pq>(*this);                                         \
    }                                                                          \
  };                                                                           \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R(int) oq ne> {                                        \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    R __VA_ARGS__(int) oq ne {                                                 \
      return invoke<D, R(int) oq ne>(static_cast<P pq>(*this), 0);             \
    }                                                                          \
  }
#define PRO4D_DEF_LHS_BINARY_OP_ACCESSOR PRO4D_DEF_MEM_ACCESSOR
#define PRO4D_DEF_LHS_ALL_OP_ACCESSOR PRO4D_DEF_MEM_ACCESSOR
#define PRO4D_LHS_LEFT_OP_DISPATCH_BODY_IMPL(...)                              \
  template <class T>                                                           \
  PRO4D_STATIC_CALL(decltype(auto), T&& self)                                  \
  PRO4D_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self))
#define PRO4D_LHS_UNARY_OP_DISPATCH_BODY_IMPL(...)                             \
  template <class T>                                                           \
  PRO4D_STATIC_CALL(decltype(auto), T&& self)                                  \
  PRO4D_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) template <class T> \
  PRO4D_STATIC_CALL(decltype(auto), T&& self, int)                             \
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__)
#define PRO4D_LHS_BINARY_OP_DISPATCH_BODY_IMPL(...)                            \
  template <class T, class Arg>                                                \
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                       \
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)                                 \
                             __VA_ARGS__ std::forward<Arg>(arg))
#define PRO4D_LHS_ALL_OP_DISPATCH_BODY_IMPL(...)                               \
  PRO4D_LHS_LEFT_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)                            \
  PRO4D_LHS_BINARY_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)
#define PRO4D_LHS_OP_DISPATCH_IMPL(type, ...)                                  \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, false> {                              \
    PRO4D_LHS_##type##_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)                      \
        PRO4D_DEF_ACCESSOR_TEMPLATE(                                           \
            MEM, PRO4D_DEF_LHS_##type##_OP_ACCESSOR, operator __VA_ARGS__)     \
  };

#define PRO4D_DEF_RHS_OP_ACCESSOR(oq, pq, ne, ...)                             \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg) oq ne> {                                        \
    friend R operator __VA_ARGS__(Arg arg, P pq self) ne {                     \
      return invoke<D, R(Arg) oq ne>(static_cast<P pq>(self),                  \
                                     std::forward<Arg>(arg));                  \
    }                                                                          \
    PRO4D_DEBUG(                                                             \
      accessor() noexcept { std::ignore = &pro_symbol_guard; }               \
                                                                             \
    private:                                                                 \
      static inline R pro_symbol_guard(Arg arg, P pq self) {                 \
        return std::forward<Arg>(arg) __VA_ARGS__ static_cast<P pq>(self);   \
      }                                                                      \
    ) \
  }
#define PRO4D_RHS_OP_DISPATCH_IMPL(...)                                        \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, true> {                               \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<Arg>(arg)                              \
                               __VA_ARGS__ std::forward<T>(self))              \
        PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PRO4D_DEF_RHS_OP_ACCESSOR,           \
                                    __VA_ARGS__)                               \
  };

#define PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL(...)                            \
  PRO4D_LHS_OP_DISPATCH_IMPL(ALL, __VA_ARGS__)                                 \
  PRO4D_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define PRO4D_BINARY_OP_DISPATCH_IMPL(...)                                     \
  PRO4D_LHS_OP_DISPATCH_IMPL(BINARY, __VA_ARGS__)                              \
  PRO4D_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define PRO4D_DEF_LHS_ASSIGNMENT_OP_ACCESSOR(oq, pq, ne, ...)                  \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg) oq ne> {                                        \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    decltype(auto) __VA_ARGS__(Arg arg) oq ne {                                \
      invoke<D, R(Arg) oq ne>(static_cast<P pq>(*this),                        \
                              std::forward<Arg>(arg));                         \
      return static_cast<P pq>(*this);                                         \
    }                                                                          \
  }
#define PRO4D_DEF_RHS_ASSIGNMENT_OP_ACCESSOR(oq, pq, ne, ...)                  \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg&) oq ne> {                                       \
    friend Arg& operator __VA_ARGS__(Arg& arg, P pq self) ne {                 \
      invoke<D, R(Arg&) oq ne>(static_cast<P pq>(self), arg);                  \
      return arg;                                                              \
    }                                                                          \
    PRO4D_DEBUG(                                                               \
        accessor() noexcept { std::ignore = &pro_symbol_guard; }               \
                                                                               \
        private : static inline Arg& pro_symbol_guard(                         \
            Arg& arg,                                                          \
            P pq self) { return arg __VA_ARGS__ static_cast<P pq>(self); })    \
  }
#define PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(...)                                 \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, false> {                              \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)                               \
                               __VA_ARGS__ std::forward<Arg>(arg))             \
        PRO4D_DEF_ACCESSOR_TEMPLATE(                                           \
            MEM, PRO4D_DEF_LHS_ASSIGNMENT_OP_ACCESSOR, operator __VA_ARGS__)   \
  };                                                                           \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, true> {                               \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<Arg>(arg)                              \
                               __VA_ARGS__ std::forward<T>(self))              \
        PRO4D_DEF_ACCESSOR_TEMPLATE(FREE,                                      \
                                    PRO4D_DEF_RHS_ASSIGNMENT_OP_ACCESSOR,      \
                                    __VA_ARGS__)                               \
  };

PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL(+)
PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL(-)
PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL(*)
PRO4D_BINARY_OP_DISPATCH_IMPL(/)
PRO4D_BINARY_OP_DISPATCH_IMPL(%)
PRO4D_LHS_OP_DISPATCH_IMPL(UNARY, ++)
PRO4D_LHS_OP_DISPATCH_IMPL(UNARY, --)
PRO4D_BINARY_OP_DISPATCH_IMPL(==)
PRO4D_BINARY_OP_DISPATCH_IMPL(!=)
PRO4D_BINARY_OP_DISPATCH_IMPL(>)
PRO4D_BINARY_OP_DISPATCH_IMPL(<)
PRO4D_BINARY_OP_DISPATCH_IMPL(>=)
PRO4D_BINARY_OP_DISPATCH_IMPL(<=)
PRO4D_BINARY_OP_DISPATCH_IMPL(<=>)
PRO4D_LHS_OP_DISPATCH_IMPL(LEFT, !)
PRO4D_BINARY_OP_DISPATCH_IMPL(&&)
PRO4D_BINARY_OP_DISPATCH_IMPL(||)
PRO4D_LHS_OP_DISPATCH_IMPL(LEFT, ~)
PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL(&)
PRO4D_BINARY_OP_DISPATCH_IMPL(|)
PRO4D_BINARY_OP_DISPATCH_IMPL(^)
PRO4D_BINARY_OP_DISPATCH_IMPL(<<)
PRO4D_BINARY_OP_DISPATCH_IMPL(>>)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(+=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(-=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(*=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(/=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(%=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(&=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(|=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(^=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(<<=)
PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL(>>=)
PRO4D_BINARY_OP_DISPATCH_IMPL(, )
PRO4D_BINARY_OP_DISPATCH_IMPL(->*)

template <>
struct operator_dispatch<"()", false> {
  template <class T, class... Args>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)(std::forward<Args>(args)...))
      PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, operator())
};
template <>
struct operator_dispatch<"[]", false> {
#if __cpp_multidimensional_subscript >= 202110L
  template <class T, class... Args>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Args>(args)...])
#else
  template <class T, class Arg>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Arg>(arg)])
#endif // __cpp_multidimensional_subscript >= 202110L
      PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, operator[])
};

#undef PRO4D_ASSIGNMENT_OP_DISPATCH_IMPL
#undef PRO4D_DEF_RHS_ASSIGNMENT_OP_ACCESSOR
#undef PRO4D_DEF_LHS_ASSIGNMENT_OP_ACCESSOR
#undef PRO4D_BINARY_OP_DISPATCH_IMPL
#undef PRO4D_EXTENDED_BINARY_OP_DISPATCH_IMPL
#undef PRO4D_RHS_OP_DISPATCH_IMPL
#undef PRO4D_DEF_RHS_OP_ACCESSOR
#undef PRO4D_LHS_OP_DISPATCH_IMPL
#undef PRO4D_LHS_ALL_OP_DISPATCH_BODY_IMPL
#undef PRO4D_LHS_BINARY_OP_DISPATCH_BODY_IMPL
#undef PRO4D_LHS_UNARY_OP_DISPATCH_BODY_IMPL
#undef PRO4D_LHS_LEFT_OP_DISPATCH_BODY_IMPL
#undef PRO4D_DEF_LHS_ALL_OP_ACCESSOR
#undef PRO4D_DEF_LHS_BINARY_OP_ACCESSOR
#undef PRO4D_DEF_LHS_UNARY_OP_ACCESSOR
#undef PRO4D_DEF_LHS_LEFT_OP_ACCESSOR

struct implicit_conversion_dispatch : detail::cast_dispatch_base<false, false> {
  template <class T>
  PRO4D_STATIC_CALL(T&&, T&& self) noexcept {
    return std::forward<T>(self);
  }
};
struct explicit_conversion_dispatch : detail::cast_dispatch_base<true, false> {
  template <class T>
  PRO4D_STATIC_CALL(auto, T&& self) noexcept {
    return detail::converter{[&self]<class U>(std::in_place_type_t<U>) noexcept(
                                 std::is_nothrow_constructible_v<U, T>) -> U
                               requires(detail::explicitly_convertible<T&&, U>)
                             { return static_cast<U>(std::forward<T>(self)); }};
  }
};
using conversion_dispatch = explicit_conversion_dispatch;

class not_implemented : public std::exception {
public:
  char const* what() const noexcept override {
    return "pro::v4::not_implemented";
  }
};

template <class D>
struct weak_dispatch : D {
  using D::operator();
  template <class... Args>
  [[noreturn]] PRO4D_STATIC_CALL(detail::wildcard, Args&&...)
    requires(!std::is_invocable_v<D, Args...>)
  {
    PRO4D_THROW(not_implemented{});
  }
};

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_DISPATCH_H_
