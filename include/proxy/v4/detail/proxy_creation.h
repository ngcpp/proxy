// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_PROXY_CREATION_H_
#define MSFT_PROXY_V4_DETAIL_PROXY_CREATION_H_

#include <initializer_list>
#include <memory>
#include <new>

#if __STDC_HOSTED__
#include <atomic>
#endif // __STDC_HOSTED__

#include "core.h"

namespace pro::inline v4 {

template <class T, class F>
concept inplace_proxiable_target = proxiable<detail::inplace_ptr<T>, F>;

template <class T, class F>
concept proxiable_target =
    proxiable<detail::observer_ptr<T&, const T&, T&&, const T&&>,
              observer_facade<F>>;

template <class T>
  requires(is_bitwise_trivially_relocatable_v<T>)
struct is_bitwise_trivially_relocatable<detail::inplace_ptr<T>>
    : std::true_type {};

template <facade F, class T>
constexpr proxy<F> make_proxy_inplace(T&& value) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<T>, T>)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{std::in_place_type<detail::inplace_ptr<std::decay_t<T>>>,
                  std::in_place, std::forward<T>(value)};
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_inplace(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<detail::inplace_ptr<T>>, std::in_place,
                  std::forward<Args>(args)...};
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F>
    make_proxy_inplace(std::initializer_list<U> il, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<detail::inplace_ptr<T>>, std::in_place, il,
                  std::forward<Args>(args)...};
}

template <facade F, class T>
constexpr proxy_view<F> make_proxy_view(T& value) noexcept {
  return proxy_view<F>{
      detail::observer_ptr<T&, const T&, T&&, const T&&>{value}};
}

#if __STDC_HOSTED__
namespace detail {

template <class T, class Alloc, class... Args>
T* allocate(const Alloc& alloc, Args&&... args) {
  auto al =
      typename std::allocator_traits<Alloc>::template rebind_alloc<T>(alloc);
  auto deleter = [&](T* ptr) { al.deallocate(ptr, 1); };
  std::unique_ptr<T, decltype(deleter)> result{al.allocate(1), deleter};
  std::construct_at(result.get(), std::forward<Args>(args)...);
  return result.release();
}
template <class Alloc, class T>
void deallocate(const Alloc& alloc, T* ptr) {
  auto al =
      typename std::allocator_traits<Alloc>::template rebind_alloc<T>(alloc);
  std::destroy_at(ptr);
  al.deallocate(ptr, 1);
}
template <class Alloc>
struct alloc_aware {
  explicit alloc_aware(const Alloc& alloc) noexcept : alloc(alloc) {}
  alloc_aware(const alloc_aware&) noexcept = default;

  [[PRO4D_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  Alloc alloc;
};
template <class T>
class indirect_ptr {
public:
  explicit indirect_ptr(T* ptr) noexcept : ptr_(ptr) {}
  auto operator->() noexcept { return std::addressof(**ptr_); }
  auto operator->() const noexcept { return std::addressof(**ptr_); }
  decltype(auto) operator*() & noexcept { return **ptr_; }
  decltype(auto) operator*() const& noexcept { return *std::as_const(*ptr_); }
  decltype(auto) operator*() && noexcept { return *std::move(*ptr_); }
  decltype(auto) operator*() const&& noexcept {
    return *std::move(std::as_const(*ptr_));
  }

protected:
  T* ptr_;
};

template <class T, class Alloc>
class PRO4D_ENFORCE_EBO wide_ptr : private alloc_aware<Alloc>,
                                   public indirect_ptr<inplace_ptr<T>> {
public:
  template <class... Args>
  wide_ptr(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        indirect_ptr<inplace_ptr<T>>(allocate<inplace_ptr<T>>(
            this->alloc, std::in_place, std::forward<Args>(args)...)) {}
  wide_ptr(const wide_ptr& rhs)
    requires(std::is_copy_constructible_v<T>)
      : alloc_aware<Alloc>(rhs),
        indirect_ptr<inplace_ptr<T>>(
            allocate<inplace_ptr<T>>(this->alloc, std::in_place, *rhs)) {}
  wide_ptr(wide_ptr&& rhs) = delete;
  ~wide_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    deallocate(this->alloc, this->ptr_);
  }
};

template <class T, class Alloc>
struct PRO4D_ENFORCE_EBO compact_ptr_storage : alloc_aware<Alloc>,
                                               inplace_ptr<T> {
  template <class... Args>
  explicit compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        inplace_ptr<T>(std::in_place, std::forward<Args>(args)...) {}
};
template <class T, class Alloc>
class compact_ptr : public indirect_ptr<compact_ptr_storage<T, Alloc>> {
  using Storage = compact_ptr_storage<T, Alloc>;

public:
  template <class... Args>
  compact_ptr(const Alloc& alloc, Args&&... args)
      : indirect_ptr<Storage>(
            allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  compact_ptr(const compact_ptr& rhs)
    requires(std::is_copy_constructible_v<T>)
      : indirect_ptr<Storage>(
            allocate<Storage>(rhs.ptr_->alloc, rhs.ptr_->alloc, *rhs)) {}
  compact_ptr(compact_ptr&& rhs) = delete;
  ~compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    deallocate(this->ptr_->alloc, this->ptr_);
  }
};

struct shared_compact_ptr_storage_base {
  std::atomic_long ref_count = 1;
};
template <class T, class Alloc>
struct PRO4D_ENFORCE_EBO shared_compact_ptr_storage
    : shared_compact_ptr_storage_base,
      alloc_aware<Alloc>,
      inplace_ptr<T> {
  template <class... Args>
  explicit shared_compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        inplace_ptr<T>(std::in_place, std::forward<Args>(args)...) {}
};
template <class T, class Alloc>
class shared_compact_ptr
    : public indirect_ptr<shared_compact_ptr_storage<T, Alloc>> {
  using Storage = shared_compact_ptr_storage<T, Alloc>;

public:
  template <class... Args>
  shared_compact_ptr(const Alloc& alloc, Args&&... args)
      : indirect_ptr<Storage>(
            allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  shared_compact_ptr(const shared_compact_ptr& rhs) noexcept
      : indirect_ptr<Storage>(rhs.ptr_) {
    this->ptr_->ref_count.fetch_add(1, std::memory_order::relaxed);
  }
  shared_compact_ptr(shared_compact_ptr&& rhs) = delete;
  ~shared_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (this->ptr_->ref_count.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      deallocate(this->ptr_->alloc, this->ptr_);
    }
  }
};

struct strong_weak_compact_ptr_storage_base {
  std::atomic_long strong_count = 1, weak_count = 1;
};
template <class T, class Alloc>
struct strong_weak_compact_ptr_storage : strong_weak_compact_ptr_storage_base,
                                         alloc_aware<Alloc> {
  template <class... Args>
  explicit strong_weak_compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc) {
    std::construct_at(reinterpret_cast<T*>(&value),
                      std::forward<Args>(args)...);
  }

  alignas(alignof(T)) std::byte value[sizeof(T)];
};
template <class T, class Alloc>
class weak_compact_ptr;
template <class T, class Alloc>
class strong_compact_ptr {
  using Storage = strong_weak_compact_ptr_storage<T, Alloc>;
  friend class weak_compact_ptr<T, Alloc>;

public:
  using weak_type = weak_compact_ptr<T, Alloc>;

  explicit strong_compact_ptr(Storage* ptr) noexcept : ptr_(ptr) {}
  template <class... Args>
  strong_compact_ptr(const Alloc& alloc, Args&&... args)
      : ptr_(allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  strong_compact_ptr(const strong_compact_ptr& rhs) noexcept : ptr_(rhs.ptr_) {
    ptr_->strong_count.fetch_add(1, std::memory_order::relaxed);
  }
  strong_compact_ptr(strong_compact_ptr&& rhs) = delete;
  ~strong_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr_->strong_count.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      std::destroy_at(operator->());
      if (ptr_->weak_count.fetch_sub(1u, std::memory_order::release) == 1) {
        deallocate(ptr_->alloc, ptr_);
      }
    }
  }
  T* operator->() noexcept {
    return std::launder(reinterpret_cast<T*>(&ptr_->value));
  }
  const T* operator->() const noexcept {
    return std::launder(reinterpret_cast<const T*>(&ptr_->value));
  }
  T& operator*() & noexcept { return *operator->(); }
  const T& operator*() const& noexcept { return *operator->(); }
  T&& operator*() && noexcept { return std::move(*operator->()); }
  const T&& operator*() const&& noexcept { return std::move(*operator->()); }

private:
  strong_weak_compact_ptr_storage<T, Alloc>* ptr_;
};
template <class T, class Alloc>
class weak_compact_ptr {
public:
  using element_type = T;

  weak_compact_ptr(const strong_compact_ptr<T, Alloc>& rhs) noexcept
      : ptr_(rhs.ptr_) {
    ptr_->weak_count.fetch_add(1, std::memory_order::relaxed);
  }
  weak_compact_ptr(const weak_compact_ptr& rhs) noexcept : ptr_(rhs.ptr_) {
    ptr_->weak_count.fetch_add(1, std::memory_order::relaxed);
  }
  weak_compact_ptr(weak_compact_ptr&& rhs) = delete;
  ~weak_compact_ptr() noexcept {
    if (ptr_->weak_count.fetch_sub(1u, std::memory_order::acq_rel) == 1) {
      deallocate(ptr_->alloc, ptr_);
    }
  }
  auto lock() const noexcept {
    return converter{[ptr = this->ptr_]<class F>(
                         std::in_place_type_t<proxy<F>>) noexcept -> proxy<F> {
      long ref_count = ptr->strong_count.load(std::memory_order::relaxed);
      do {
        if (ref_count == 0) {
          return proxy<F>{};
        }
      } while (!ptr->strong_count.compare_exchange_weak(
          ref_count, ref_count + 1, std::memory_order::relaxed));
      return proxy<F>{std::in_place_type<strong_compact_ptr<T, Alloc>>, ptr};
    }};
  }

private:
  strong_weak_compact_ptr_storage<T, Alloc>* ptr_;
};

template <class F, class T, class Alloc>
struct allocated_ptr_traits : std::type_identity<compact_ptr<T, Alloc>> {};
template <class F, class T, class Alloc>
  requires(proxiable<wide_ptr<T, Alloc>, F>)
struct allocated_ptr_traits<F, T, Alloc>
    : std::type_identity<wide_ptr<T, Alloc>> {};
template <class F, class T, class Alloc>
using allocated_ptr = allocated_ptr_traits<F, T, Alloc>::type;

template <class F, class T>
struct owned_ptr_traits : allocated_ptr_traits<F, T, std::allocator<void>> {};
template <class F, class T>
  requires(proxiable<inplace_ptr<T>, F>)
struct owned_ptr_traits<F, T> : std::type_identity<inplace_ptr<T>> {};
template <class F, class T>
using owned_ptr = owned_ptr_traits<F, T>::type;

template <class F, class T, class Alloc>
struct shared_ptr_traits : std::type_identity<shared_compact_ptr<T, Alloc>> {};
template <class F, class T, class Alloc>
  requires(std::is_convertible_v<proxy<F>, weak_proxy<F>>)
struct shared_ptr_traits<F, T, Alloc>
    : std::type_identity<strong_compact_ptr<T, Alloc>> {};
template <class F, class T, class Alloc = std::allocator<void>>
using shared_ptr = shared_ptr_traits<F, T, Alloc>::type;

} // namespace detail

template <class T, class D>
  requires(is_bitwise_trivially_relocatable_v<D>)
struct is_bitwise_trivially_relocatable<std::unique_ptr<T, D>>
    : std::true_type {};
template <class T>
struct is_bitwise_trivially_relocatable<std::shared_ptr<T>> : std::true_type {};
template <class T>
struct is_bitwise_trivially_relocatable<std::weak_ptr<T>> : std::true_type {};
template <class T, class Alloc>
  requires(is_bitwise_trivially_relocatable_v<Alloc>)
struct is_bitwise_trivially_relocatable<detail::wide_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<detail::compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<detail::shared_compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<detail::strong_compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<detail::weak_compact_ptr<T, Alloc>>
    : std::true_type {};

template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{
      std::in_place_type<detail::allocated_ptr<F, std::decay_t<T>, Alloc>>,
      alloc, std::forward<T>(value)};
}
template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<detail::allocated_ptr<F, T, Alloc>>, alloc,
                  std::forward<Args>(args)...};
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy(const Alloc& alloc,
                                  std::initializer_list<U> il, Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<detail::allocated_ptr<F, T, Alloc>>, alloc,
                  il, std::forward<Args>(args)...};
}
template <facade F, class T>
constexpr proxy<F> make_proxy(T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{std::in_place_type<detail::owned_ptr<F, std::decay_t<T>>>,
                  std::allocator<void>{}, std::forward<T>(value)};
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy(Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<detail::owned_ptr<F, T>>,
                  std::allocator<void>{}, std::forward<Args>(args)...};
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<detail::owned_ptr<F, T>>,
                  std::allocator<void>{}, il, std::forward<Args>(args)...};
}

template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{
      std::in_place_type<detail::shared_ptr<F, std::decay_t<T>, Alloc>>, alloc,
      std::forward<T>(value)};
}
template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<detail::shared_ptr<F, T, Alloc>>, alloc,
                  std::forward<Args>(args)...};
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc,
                                         std::initializer_list<U> il,
                                         Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<detail::shared_ptr<F, T, Alloc>>, alloc,
                  il, std::forward<Args>(args)...};
}
template <facade F, class T>
constexpr proxy<F> make_proxy_shared(T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{std::in_place_type<detail::shared_ptr<F, std::decay_t<T>>>,
                  std::allocator<void>{}, std::forward<T>(value)};
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_shared(Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<detail::shared_ptr<F, T>>,
                  std::allocator<void>{}, std::forward<Args>(args)...};
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy_shared(std::initializer_list<U> il,
                                     Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<detail::shared_ptr<F, T>>,
                  std::allocator<void>{}, il, std::forward<Args>(args)...};
}
#endif // __STDC_HOSTED__

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_PROXY_CREATION_H_
