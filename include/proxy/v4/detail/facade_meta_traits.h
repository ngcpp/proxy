// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_FACADE_META_TRAITS_H_
#define MSFT_PROXY_V4_DETAIL_FACADE_META_TRAITS_H_

#include <cstddef>
#include <type_traits>
#include <utility>

#ifdef __has_feature
#if __has_feature(ptrauth_calls)
#include <ptrauth.h>
#define PRO4D_HAS_PAC
#endif // __has_feature(ptrauth_calls)
#endif // __has_feature

#include "../proxy_macros.h"

namespace pro::inline v4 {

namespace detail {

#ifdef PRO4D_HAS_PAC
template <class O, class Disc>
class code_ptr {
public:
  code_ptr() = default;
  template <class F>
  explicit code_ptr(const F& f) noexcept
      : p_(ptrauth_sign_unauthenticated(
            ptrauth_strip(static_cast<O*>(f), ptrauth_key_function_pointer),
            ptrauth_key_function_pointer, schema())) {}
  code_ptr(const code_ptr& rhs) noexcept
      : p_(ptrauth_auth_and_resign(rhs.p_, ptrauth_key_function_pointer,
                                   rhs.schema(), ptrauth_key_function_pointer,
                                   schema())) {}
  code_ptr& operator=(const code_ptr& rhs) noexcept {
    p_ = ptrauth_auth_and_resign(rhs.p_, ptrauth_key_function_pointer,
                                 rhs.schema(), ptrauth_key_function_pointer,
                                 schema());
    return *this;
  }
  code_ptr& operator=(std::nullptr_t) noexcept {
    p_ = nullptr;
    return *this;
  }
  bool operator==(std::nullptr_t) const noexcept { return p_ == nullptr; }
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const {
    return ptrauth_auth_function(p_, ptrauth_key_function_pointer,
                                 schema())(std::forward<Args>(args)...);
  }

private:
  ptrauth_extra_data_t schema() const noexcept {
    return ptrauth_blend_discriminator(&p_, ptrauth_type_discriminator(Disc));
  }

  O* p_;
};

template <class T, class Disc>
class meta_ptr {
public:
  meta_ptr() = default;
  explicit meta_ptr(const T* p) noexcept
      : p_(ptrauth_sign_unauthenticated(p, ptrauth_key_cxx_vtable_pointer,
                                        schema())) {}
  meta_ptr(const meta_ptr& rhs) noexcept
      : p_(ptrauth_auth_and_resign(rhs.p_, ptrauth_key_cxx_vtable_pointer,
                                   rhs.schema(), ptrauth_key_cxx_vtable_pointer,
                                   schema())) {}
  meta_ptr& operator=(const meta_ptr& rhs) noexcept {
    p_ = ptrauth_auth_and_resign(rhs.p_, ptrauth_key_cxx_vtable_pointer,
                                 rhs.schema(), ptrauth_key_cxx_vtable_pointer,
                                 schema());
    return *this;
  }
  meta_ptr& operator=(std::nullptr_t) noexcept {
    p_ = nullptr;
    return *this;
  }
  bool operator==(std::nullptr_t) const noexcept { return p_ == nullptr; }
  const T& operator*() const noexcept {
    return *ptrauth_auth_data(p_, ptrauth_key_cxx_vtable_pointer, schema());
  }

private:
  ptrauth_extra_data_t schema() const noexcept {
    return ptrauth_blend_discriminator(&p_, ptrauth_type_discriminator(Disc));
  }

  const T* p_;
};
#else
template <class O, class Disc>
using code_ptr = O*;

template <class T, class Disc>
using meta_ptr = const T*;
#endif // PRO4D_HAS_PAC

template <class T>
concept nullable = requires(T v, const T cv) {
  { v.reset() } noexcept;
  { cv.has_value() } noexcept -> std::same_as<bool>;
};

template <class O, class Disc>
struct invoker_base {
  invoker_base() = default;
  template <class F>
  constexpr explicit invoker_base(const F& f) : p_(f) {}
  void reset() noexcept { p_ = nullptr; }
  bool has_value() const noexcept { return p_ != nullptr; }
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const {
    return p_(std::forward<Args>(args)...);
  }

private:
  code_ptr<O, Disc> p_;
};

template <class Ctx, class O>
struct invoker;
#define PRO4D_DEF_INVOKER(oq, pq, ne, ...)                                     \
  template <class Ctx, class R, class... Args>                                 \
  struct invoker<Ctx, R(Args...) oq ne>                                        \
      : invoker_base<R(Ctx, Args...) ne, R (*)(Ctx, Args...) ne> {             \
    invoker() = default;                                                       \
    template <class P>                                                         \
    constexpr explicit invoker(std::in_place_type_t<P>)                        \
        : invoker_base<R(Ctx, Args...) ne, R (*)(Ctx, Args...) ne>(            \
              [](Ctx ctx, Args... args) ne -> R {                              \
                return invoke<P>(ctx, std::forward<Args>(args)...);            \
              }) {}                                                            \
  }
PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(PRO4D_DEF_INVOKER)
#undef PRO4D_DEF_INVOKER

struct sentinel_meta {
  sentinel_meta() = default;
  template <class P>
  explicit sentinel_meta(std::in_place_type_t<P>) noexcept : v_(1) {}
  void reset() noexcept { v_ = 0; }
  bool has_value() const noexcept { return v_; }

private:
  std::ptrdiff_t v_;
};

template <nullable First, class... Rest>
struct PRO4D_ENFORCE_EBO inline_meta_storage : First, Rest... {
  using First::has_value;
  using First::reset;

  constexpr inline_meta_storage() noexcept {}
  template <class P>
  constexpr explicit inline_meta_storage(std::in_place_type_t<P>)
      : First(std::in_place_type<P>), Rest(std::in_place_type<P>)... {}
  inline_meta_storage(const inline_meta_storage& rhs) noexcept
      : inline_meta_storage() {
    if (static_cast<const First&>(rhs).has_value()) {
      static_cast<First&>(*this) = static_cast<const First&>(rhs);
      ((static_cast<Rest&>(*this) = static_cast<const Rest&>(rhs)), ...);
    } else {
      static_cast<First&>(*this).reset();
    }
  }
  inline_meta_storage& operator=(const inline_meta_storage& rhs) noexcept {
    if (static_cast<const First&>(rhs).has_value()) {
      static_cast<First&>(*this) = static_cast<const First&>(rhs);
      ((static_cast<Rest&>(*this) = static_cast<const Rest&>(rhs)), ...);
    } else {
      static_cast<First&>(*this).reset();
    }
    return *this;
  }
  template <class M>
  const M& get() const noexcept {
    return static_cast<const M&>(*this);
  }
};
template <nullable First>
struct inline_meta_storage<First> : First {
  using First::First;

  template <class M>
  const M& get() const noexcept {
    return static_cast<const M&>(*this);
  }
};

template <class... Ms>
struct static_meta_storage {
  static_meta_storage() = default;
  template <class P>
  explicit static_meta_storage(std::in_place_type_t<P>)
      : ptr_(std::addressof(storage<P>)) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  template <class M>
  const M& get() const noexcept {
    return (*ptr_).template get<M>();
  }

private:
  meta_ptr<inline_meta_storage<Ms...>, void (*)(Ms...)> ptr_;

  template <class P>
  static inline const inline_meta_storage<Ms...> storage{std::in_place_type<P>};
};

template <class... Ms>
struct compact_meta_storage_traits
    : std::type_identity<static_meta_storage<Ms...>> {};
template <nullable M>
struct compact_meta_storage_traits<M>
    : std::type_identity<inline_meta_storage<M>> {};
template <>
struct compact_meta_storage_traits<>
    : std::type_identity<inline_meta_storage<sentinel_meta>> {};

template <class... Ms>
struct flat_meta_storage_traits
    : std::type_identity<inline_meta_storage<sentinel_meta, Ms...>> {};
template <nullable M, class... Ms>
struct flat_meta_storage_traits<M, Ms...>
    : std::type_identity<inline_meta_storage<M, Ms...>> {};

} // namespace detail

struct compact_facade_meta_traits {
  template <class Ctx, class O>
  using invoker = detail::invoker<Ctx, O>;

  template <class... Ms>
  using storage = detail::compact_meta_storage_traits<Ms...>::type;
};

struct flat_facade_meta_traits {
  template <class Ctx, class O>
  using invoker = detail::invoker<Ctx, O>;

  template <class... Ms>
  using storage = detail::flat_meta_storage_traits<Ms...>::type;
};

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_FACADE_META_TRAITS_H_
