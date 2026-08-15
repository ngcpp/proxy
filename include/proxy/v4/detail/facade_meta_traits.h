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
  meta_ptr& operator=(const T* p) noexcept {
    p_ = ptrauth_sign_unauthenticated(p, ptrauth_key_cxx_vtable_pointer,
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

template <class M>
struct PRO4D_ENFORCE_EBO inplace_meta_storage : M {
  using M::M;

  inplace_meta_storage() = default;
  inplace_meta_storage(const inplace_meta_storage&) = default;
  template <class M2>
    requires(std::is_nothrow_convertible_v<const M2&, const M&>)
  inplace_meta_storage(const inplace_meta_storage<M2>& rhs) noexcept
      : M(static_cast<const M&>(*rhs)) {}
  inplace_meta_storage& operator=(const inplace_meta_storage&) = default;
  template <class M2>
    requires(std::is_nothrow_convertible_v<const M2&, const M&>)
  inplace_meta_storage&
      operator=(const inplace_meta_storage<M2>& rhs) noexcept {
    static_cast<M&>(*this) = static_cast<const M&>(*rhs);
    return *this;
  }

  const M& operator*() const noexcept { return *this; }
};

template <class M>
struct static_meta_storage {
  static_meta_storage() = default;
  template <class M2>
    requires(std::is_nothrow_convertible_v<const M2&, const M&>)
  static_meta_storage(const static_meta_storage<M2>& rhs) noexcept
      : ptr_(std::addressof(static_cast<const M&>(*rhs))) {}
  template <class M2>
    requires(std::is_nothrow_convertible_v<const M2&, const M&>)
  static_meta_storage& operator=(const static_meta_storage<M2>& rhs) noexcept {
    ptr_ = std::addressof(static_cast<const M&>(*rhs));
    return *this;
  }
  template <class P>
  explicit static_meta_storage(std::in_place_type_t<P>)
      : ptr_(std::addressof(storage<P>)) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  const M& operator*() const noexcept { return *ptr_; }

private:
  meta_ptr<M, void (*)(M)> ptr_;

  template <class P>
  static inline const M storage{std::in_place_type<P>};
};

} // namespace detail

struct compact_facade_meta_traits {
  template <class Ctx, class O>
  using invoker = detail::invoker<Ctx, O>;

  template <class M>
  using storage = std::conditional_t<sizeof(M) <= sizeof(void*),
                                     detail::inplace_meta_storage<M>,
                                     detail::static_meta_storage<M>>;
};

struct flat_facade_meta_traits {
  template <class Ctx, class O>
  using invoker = detail::invoker<Ctx, O>;

  template <class M>
  using storage = detail::inplace_meta_storage<M>;
};

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_FACADE_META_TRAITS_H_
