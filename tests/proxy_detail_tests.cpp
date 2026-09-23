// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <proxy/proxy.h>

namespace proxy_detail_tests_detail {

struct Base {
  int v;
};
struct Derived : Base {};

static_assert(pro::detail::explicitly_convertible<int, int>);
static_assert(pro::detail::explicitly_convertible<long, int>);
static_assert(!pro::detail::explicitly_convertible<int, int&&>);
static_assert(!pro::detail::explicitly_convertible<int, const int&>);
static_assert(pro::detail::explicitly_convertible<int&&, int&&>);
static_assert(pro::detail::explicitly_convertible<int&&, const int&>);
static_assert(!pro::detail::explicitly_convertible<long&&, int&&>);
static_assert(!pro::detail::explicitly_convertible<long, int&&>);
static_assert(pro::detail::explicitly_convertible<Derived&, Base&>);
static_assert(pro::detail::explicitly_convertible<Derived&, const Base&>);
static_assert(!pro::detail::explicitly_convertible<Derived&, Base&&>);
static_assert(!pro::detail::explicitly_convertible<const Derived&, Base&>);
static_assert(pro::detail::explicitly_convertible<const Derived&, const Base&>);
static_assert(pro::detail::explicitly_convertible<Derived, Base>);
static_assert(!pro::detail::explicitly_convertible<Derived, Base&&>);
static_assert(!pro::detail::explicitly_convertible<Base&, Derived&>);

template <int I>
struct NullableMeta {
  NullableMeta() = default;
  template <class P>
  constexpr explicit NullableMeta(std::in_place_type_t<P>) noexcept
      : v(I + 1) {}
  explicit operator bool() const noexcept { return v != 0; }

  int v = 0;
};
template <int I>
struct PlainMeta {
  PlainMeta() = default;
  template <class P>
  constexpr explicit PlainMeta(std::in_place_type_t<P>) noexcept {}
};

static_assert(pro::detail::nullable<NullableMeta<0>>);
static_assert(!pro::detail::nullable<PlainMeta<0>>);

struct PolicyWithoutInvoker {
  template <class M>
  using storage = pro::detail::inline_meta_storage<M>;
};
struct PolicyWithoutStorage {
  template <class Ctx, class O>
  using invoker = pro::detail::invoker<Ctx, O>;
};
template <class Ctx, class O>
struct FinalInvoker final : pro::detail::invoker<Ctx, O> {
  using pro::detail::invoker<Ctx, O>::invoker;
};
struct PolicyWithFinalInvoker {
  template <class Ctx, class O>
  using invoker = FinalInvoker<Ctx, O>;
  template <class M>
  using storage = pro::detail::inline_meta_storage<M>;
};
template <class Ctx, class O>
struct PlainInvoker {
  PlainInvoker() = default;
  template <class P>
  constexpr explicit PlainInvoker(std::in_place_type_t<P>) noexcept {}
};
struct PolicyWithPlainInvoker {
  template <class Ctx, class O>
  using invoker = PlainInvoker<Ctx, O>;
  template <class M>
  using storage = pro::detail::inline_meta_storage<M>;
};
template <class M>
struct PlainStorage {
  PlainStorage() = default;
  template <class P>
  explicit PlainStorage(std::in_place_type_t<P>) noexcept : value_(nullptr) {}
  const M& operator*() const noexcept { return value_; }

  M value_;
};
struct PolicyWithPlainStorage {
  template <class Ctx, class O>
  using invoker = pro::detail::invoker<Ctx, O>;
  template <class M>
  using storage = PlainStorage<M>;
};
template <class M>
struct ByValueStorage : M {
  using M::M;
  M operator*() const noexcept { return *this; }
};
struct PolicyWithByValueStorage {
  template <class Ctx, class O>
  using invoker = pro::detail::invoker<Ctx, O>;
  template <class M>
  using storage = ByValueStorage<M>;
};

static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithoutInvoker>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithoutStorage>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithFinalInvoker>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithPlainInvoker>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithPlainStorage>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithByValueStorage>());

using M0 = NullableMeta<0>;
using M1 = NullableMeta<1>;
using M2 = NullableMeta<2>;
using M01 = pro::detail::proxy_meta_base_t<M0, M1>;
using M02 = pro::detail::proxy_meta_base_t<M0, M2>;

static_assert(std::is_same_v<
              pro::detail::proxy_meta_base_t<>,
              pro::detail::proxy_meta_base_impl<pro::detail::sentinel_meta>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<PlainMeta<0>>,
                             pro::detail::proxy_meta_base_impl<
                                 pro::detail::sentinel_meta, PlainMeta<0>>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0>,
                             pro::detail::proxy_meta_base_impl<M0>>);

static_assert(std::is_base_of_v<M0, pro::detail::proxy_meta_base_t<M0>>);
static_assert(!std::is_base_of_v<M0, M01>);

static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0, M0>,
                             pro::detail::proxy_meta_base_impl<M0>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0, M1>,
                             pro::detail::proxy_meta_base_impl<M0, M1>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0, M01>,
                             pro::detail::proxy_meta_base_impl<M01>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M01, M0>,
                             pro::detail::proxy_meta_base_impl<M01>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0, M2, M01>,
                             pro::detail::proxy_meta_base_impl<M01, M2>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<M0, M01, M02>,
                             pro::detail::proxy_meta_base_impl<M01, M02>>);
static_assert(
    std::is_same_v<pro::detail::proxy_meta_base_t<
                       M0, M01, pro::detail::proxy_meta_base_t<M01, M2>>,
                   pro::detail::proxy_meta_base_impl<
                       pro::detail::proxy_meta_base_t<M01, M2>>>);

static_assert(std::is_nothrow_convertible_v<const M01&, const M0&>);
static_assert(std::is_nothrow_convertible_v<const M01&, const M1&>);
static_assert(!std::is_nothrow_convertible_v<const M01&, const M2&>);
static_assert(!std::is_nothrow_convertible_v<const M0&, const M01&>);

inline constexpr pro::detail::proxy_meta_base_t<M1, M0> kReordered{
    std::in_place_type<int>};
inline constexpr pro::detail::proxy_meta_base_t<M0, M01, M02> kDiamond{
    std::in_place_type<int>};

static_assert(static_cast<const M0&>(kReordered).v == 1);
static_assert(static_cast<const M1&>(kReordered).v == 2);
static_assert(
    std::addressof(static_cast<const M0&>(kDiamond)) ==
    std::addressof(static_cast<const M0&>(static_cast<const M01&>(kDiamond))));
static_assert(
    std::addressof(static_cast<const M2&>(kDiamond)) ==
    std::addressof(static_cast<const M2&>(static_cast<const M02&>(kDiamond))));

} // namespace proxy_detail_tests_detail
