// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <proxy/proxy.h>

namespace proxy_metadata_policy_tests_detail {

template <std::size_t N>
using Tag = std::integral_constant<std::size_t, N>;

template <std::size_t N>
std::size_t ScaleAndAdd(std::size_t value, std::size_t arg) noexcept {
  return value * N + arg;
}

struct ThrowingMove {
  ThrowingMove() = default;
  ThrowingMove(ThrowingMove&&) noexcept(false) {}
};

using Invoker = pro::detail::code_ptr<std::size_t(std::size_t, std::size_t)>;
using NoexceptInvoker =
    pro::detail::code_ptr<std::size_t(std::size_t, std::size_t) noexcept>;

static_assert(
    std::is_same_v<
        pro::compact_metadata::invoker<std::size_t(std::size_t, std::size_t)>,
        Invoker>);
static_assert(
    std::is_same_v<
        pro::inline_metadata::invoker<std::size_t(std::size_t, std::size_t)>,
        Invoker>);
static_assert(
    std::is_nothrow_constructible_v<Invoker, decltype(ScaleAndAdd<1>)&>);
static_assert(std::is_nothrow_invocable_v<const NoexceptInvoker&, std::size_t,
                                          std::size_t>);
static_assert(
    !std::is_nothrow_invocable_v<const Invoker&, std::size_t, std::size_t>);
static_assert(!std::is_invocable_v<const Invoker&, std::size_t>);
static_assert(!std::is_nothrow_invocable_v<
              const pro::detail::code_ptr<void(ThrowingMove) noexcept>&,
              ThrowingMove>);
static_assert(
    std::is_same_v<
        std::invoke_result_t<const pro::detail::code_ptr<int&(int&)>&, int&>,
        int&>);
static_assert(std::is_invocable_v<
              const pro::detail::code_ptr<void(std::unique_ptr<int>)>&,
              std::unique_ptr<int>>);

struct ScaleDispatch {
  template <class P>
  std::size_t operator()(const P&, std::size_t arg) const noexcept {
    return P::value * arg;
  }
};
using ScaleMeta =
    pro::detail::invocation_meta<pro::inline_metadata, true, ScaleDispatch,
                                 std::size_t(std::size_t) const noexcept>;

struct TagMeta {
  template <class P>
  explicit TagMeta(std::in_place_type_t<P>) noexcept : value(P::value) {}
  TagMeta(const TagMeta&) = default;
  TagMeta& operator=(const TagMeta&) = delete;

  std::size_t value;
};
using SmallMeta = pro::detail::proxy_meta_base_t<ScaleMeta>;
using LargeMeta = pro::detail::proxy_meta_base_impl<SmallMeta, TagMeta>;

static_assert(sizeof(SmallMeta) <= sizeof(void*));
static_assert(sizeof(LargeMeta) > sizeof(void*));
static_assert(std::is_same_v<pro::compact_metadata::storage<SmallMeta>,
                             pro::detail::inline_meta_storage<SmallMeta>>);
static_assert(std::is_same_v<pro::compact_metadata::storage<LargeMeta>,
                             pro::detail::static_meta_storage<LargeMeta>>);
static_assert(std::is_same_v<pro::inline_metadata::storage<SmallMeta>,
                             pro::detail::inline_meta_storage<SmallMeta>>);
static_assert(std::is_same_v<pro::inline_metadata::storage<LargeMeta>,
                             pro::detail::inline_meta_storage<LargeMeta>>);

template <class F>
class MinimalInvoker {
public:
  MinimalInvoker() = default;
  template <class C>
  explicit MinimalInvoker(const C& c) noexcept : f_(c) {}
  MinimalInvoker(const MinimalInvoker&) = default;
  MinimalInvoker(MinimalInvoker&&) = delete;
  MinimalInvoker& operator=(const MinimalInvoker&) = default;
  explicit operator bool() const noexcept { return f_ != nullptr; }
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const
      noexcept(std::is_nothrow_invocable_v<F*, Args...>) {
    return f_(std::forward<Args>(args)...);
  }

private:
  F* f_;
};

template <class M>
class MinimalStorage {
public:
  MinimalStorage() = default;
  MinimalStorage(const MinimalStorage&) = default;
  MinimalStorage(MinimalStorage&) = delete;
  MinimalStorage(MinimalStorage&&) = delete;
  MinimalStorage& operator=(const MinimalStorage&) = default;
  MinimalStorage& operator=(MinimalStorage&&) = delete;
  template <class T>
  MinimalStorage& operator=(T&) = delete;
  template <class M2>
    requires(std::is_nothrow_convertible_v<const M2&, const M&>)
  MinimalStorage& operator=(const MinimalStorage<M2>& rhs) noexcept {
    value_ = *rhs;
    return *this;
  }
  void reset() noexcept { std::construct_at(std::addressof(value_)); }
  template <class P>
  void emplace(std::in_place_type_t<P> tag) noexcept {
    std::construct_at(std::addressof(value_), tag);
  }
  explicit operator bool() const noexcept { return static_cast<bool>(value_); }
  const M& operator*() const noexcept { return value_; }

private:
  M value_;
};

struct MinimalMetadata {
  template <class F>
  using invoker = MinimalInvoker<F>;
  template <class M>
  using storage = MinimalStorage<M>;
};

static_assert(
    pro::detail::is_metadata_policy_well_formed<pro::compact_metadata>());
static_assert(
    pro::detail::is_metadata_policy_well_formed<pro::inline_metadata>());
static_assert(pro::detail::is_metadata_policy_well_formed<MinimalMetadata>());

struct Value {
  Value(int value, int& destructions) noexcept
      : value(value), destructions(&destructions) {}
  ~Value() noexcept { ++*destructions; }

  int value;
  int* destructions;
};

struct Read {
  int operator()(const Value& self) const noexcept { return self.value; }
};
struct ReadNext {
  int operator()(const Value& self) const noexcept { return self.value + 1; }
};
struct Consume {
  template <class P>
  int operator()(P&& self) const noexcept {
    return self->value;
  }
};
struct Fail {
  template <class P>
  void operator()(P&&) const {
    throw std::runtime_error("dispatch");
  }
};
struct TypeSize {
  template <class T>
  explicit TypeSize(std::in_place_type_t<T>) noexcept : value(sizeof(T)) {}
  TypeSize(const TypeSize&) = default;
  TypeSize& operator=(const TypeSize&) = delete;

  std::size_t value;
};

struct Super : pro::facade_builder                               //
               ::add_convention<Read, int() const noexcept>      //
               ::support_copy<pro::constraint_level::nontrivial> //
               ::build {};
struct Derived : pro::facade_builder                                 //
                 ::add_facade<Super>                                 //
                 ::add_convention<ReadNext, int() const noexcept>    //
                 ::add_reflection<TypeSize>                          //
                 ::add_direct_convention<Consume, int() && noexcept> //
                 ::add_direct_convention<Fail, void() &&>            //
                 ::add_skill<pro::skills::as_view>                   //
                 ::add_skill<pro::skills::as_weak>                   //
                 ::build {};
struct Relocatable : pro::facade_builder                                     //
                     ::add_convention<Read, int() const noexcept>            //
                     ::support_relocation<pro::constraint_level::nontrivial> //
                     ::build {};

template <class MP>
concept PolicyPreservingSkills =
    std::is_convertible_v<pro::proxy<Derived, MP>&,
                          pro::proxy_view<Derived, MP>> &&
    std::is_convertible_v<const pro::proxy<Derived, MP>&,
                          pro::weak_proxy<Derived, MP>> &&
    std::is_same_v<
        decltype(std::declval<const pro::weak_proxy<Derived, MP>&>().lock()),
        pro::proxy<Derived, MP>>;
static_assert(PolicyPreservingSkills<pro::compact_metadata>);
static_assert(PolicyPreservingSkills<pro::inline_metadata>);
static_assert(PolicyPreservingSkills<MinimalMetadata>);

} // namespace proxy_metadata_policy_tests_detail

namespace detail = proxy_metadata_policy_tests_detail;

TEST(ProxyMetadataPolicyTests, TestMinimalMetadata) {
  using Proxy = pro::proxy<detail::Derived, detail::MinimalMetadata>;
  using ConsumeOverload = int() && noexcept;
  using FailOverload = void() &&;
  int destructions = 0;
  {
    Proxy p1 = std::make_shared<detail::Value>(1, destructions);
    ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*p1)), 1);
    ASSERT_EQ((invoke<detail::ReadNext, int() const noexcept>(*p1)), 2);
    ASSERT_EQ(reflect<detail::TypeSize>(*p1).value, sizeof(detail::Value));
    Proxy copy = p1;
    Proxy empty;
    Proxy empty_copy = empty;
    ASSERT_FALSE(empty_copy);
    Proxy p2 = pro::make_proxy_shared<detail::Derived, detail::Value,
                                      detail::MinimalMetadata>(2, destructions);
    Proxy moved = std::move(p2);
    ASSERT_FALSE(p2);
    swap(p1, moved);
    ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*p1)), 2);
    pro::proxy_view<detail::Derived, detail::MinimalMetadata> view = p1;
    ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*view)), 2);
    pro::weak_proxy<detail::Derived, detail::MinimalMetadata> weak = p1;
    ASSERT_TRUE(weak.lock());
    pro::proxy<detail::Super, detail::MinimalMetadata> super = copy;
    ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*super)), 1);
    super = std::move(p1);
    ASSERT_FALSE(p1);
    ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*super)), 2);
    super = empty;
    ASSERT_FALSE(super);
    ASSERT_EQ(destructions, 1);
    ASSERT_FALSE(weak.lock());
    ASSERT_EQ((invoke<detail::Consume, ConsumeOverload>(std::move(copy))), 1);
    ASSERT_FALSE(copy);
    ASSERT_THROW((invoke<detail::Fail, FailOverload>(std::move(moved))),
                 std::runtime_error);
    ASSERT_FALSE(moved);
    ASSERT_EQ(destructions, 2);
  }
  pro::proxy<detail::Relocatable, detail::MinimalMetadata> relocatable =
      std::make_shared<detail::Value>(3, destructions);
  auto relocated = std::move(relocatable);
  ASSERT_FALSE(relocatable);
  swap(relocatable, relocated);
  ASSERT_FALSE(relocated);
  ASSERT_EQ((invoke<detail::Read, int() const noexcept>(*relocatable)), 3);
  relocatable.reset();
  ASSERT_EQ(destructions, 3);
}

TEST(ProxyMetadataPolicyTests, TestCompactMetadata_DynamicInitialization) {
  int destructions = 0;
  pro::proxy<detail::Derived> p =
      std::make_shared<detail::Value>(5, destructions);
  ASSERT_EQ(reflect<detail::TypeSize>(*p).value, sizeof(detail::Value));
}
