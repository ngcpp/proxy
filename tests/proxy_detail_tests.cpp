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
struct SlotMeta {
  SlotMeta() = default;
  template <class P>
  constexpr explicit SlotMeta(std::in_place_type_t<P>) noexcept : v(I + 1) {}
  explicit operator bool() const noexcept { return v != 0; }

  int v = 0;
};
template <int I>
struct PlainMeta {
  PlainMeta() = default;
  template <class P>
  constexpr explicit PlainMeta(std::in_place_type_t<P>) noexcept {}
};

template <template <class> class I>
struct PolicyWithInvoker : pro::compact_metadata {
  template <class F>
  using invoker = I<F>;
};
template <template <class> class S>
struct PolicyWithStorage : pro::compact_metadata {
  template <class M>
  using storage = S<M>;
};

struct PolicyWithoutInvoker {
  template <class M>
  using storage = pro::detail::inline_meta_storage<M>;
};
struct PolicyWithoutStorage {
  template <class F>
  using invoker = F*;
};

template <class F>
class ClassInvoker {
public:
  ClassInvoker() = default;
  template <class C>
  explicit ClassInvoker(const C& c) noexcept : f_(c) {}
  explicit operator bool() const noexcept { return f_ != nullptr; }
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const
      noexcept(std::is_nothrow_invocable_v<F*, Args...>) {
    return f_(std::forward<Args>(args)...);
  }

private:
  F* f_ = nullptr;
};
template <class F>
struct ThrowingDefaultConstructionInvoker : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  ThrowingDefaultConstructionInvoker() noexcept(false) {}
};
template <class F>
struct ThrowingConstructionInvoker : ClassInvoker<F> {
  ThrowingConstructionInvoker() = default;
  template <class C>
  explicit ThrowingConstructionInvoker(const C& c) : ClassInvoker<F>(c) {}
};
template <class F>
struct ThrowingCopyInvoker : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  ThrowingCopyInvoker() = default;
  ThrowingCopyInvoker(const ThrowingCopyInvoker& rhs) noexcept(false)
      : ClassInvoker<F>(rhs) {}
};
template <class F>
struct ThrowingCopyAssignmentInvoker : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  ThrowingCopyAssignmentInvoker() = default;
  ThrowingCopyAssignmentInvoker(const ThrowingCopyAssignmentInvoker&) = default;
  ThrowingCopyAssignmentInvoker&
      operator=(const ThrowingCopyAssignmentInvoker& rhs) noexcept(false) {
    ClassInvoker<F>::operator=(rhs);
    return *this;
  }
};
template <class F>
struct NonTrivialDestructionInvoker : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  ~NonTrivialDestructionInvoker() noexcept {}
};
template <class F>
struct InvokerWithoutBoolConversion : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  explicit operator bool() const = delete;
};
template <class F>
struct UncallableInvoker : ClassInvoker<F> {
  using ClassInvoker<F>::ClassInvoker;
  void operator()(void*) const = delete;
};

template <class M>
struct ThrowingCopyStorage : pro::detail::inline_meta_storage<M> {
  ThrowingCopyStorage() = default;
  ThrowingCopyStorage(const ThrowingCopyStorage& rhs) noexcept(false)
      : pro::detail::inline_meta_storage<M>(rhs) {}
  ThrowingCopyStorage& operator=(const ThrowingCopyStorage&) = default;
};
template <class M>
struct ThrowingDefaultConstructionStorage
    : pro::detail::inline_meta_storage<M> {
  ThrowingDefaultConstructionStorage() noexcept(false) {}
};
template <class M>
struct ThrowingCopyAssignmentStorage : pro::detail::inline_meta_storage<M> {
  ThrowingCopyAssignmentStorage() = default;
  ThrowingCopyAssignmentStorage(const ThrowingCopyAssignmentStorage&) = default;
  ThrowingCopyAssignmentStorage&
      operator=(const ThrowingCopyAssignmentStorage&) noexcept(false) {
    return *this;
  }
};
template <class M>
struct NonTrivialDestructionStorage : pro::detail::inline_meta_storage<M> {
  ~NonTrivialDestructionStorage() noexcept {}
};
template <class M>
struct ThrowingDestructionStorage : pro::detail::inline_meta_storage<M> {
  ~ThrowingDestructionStorage() noexcept(false) {}
};
template <class M>
struct ThrowingResetStorage : pro::detail::inline_meta_storage<M> {
  void reset() {}
};
template <class M>
struct ThrowingEmplaceStorage : pro::detail::inline_meta_storage<M> {
  template <class P>
  void emplace(std::in_place_type_t<P>) {}
};
template <class M>
struct StorageWithoutBoolConversion : pro::detail::inline_meta_storage<M> {
  explicit operator bool() const = delete;
};
template <class M>
struct ByValueStorage : pro::detail::inline_meta_storage<M> {
  M operator*() const noexcept {
    return pro::detail::inline_meta_storage<M>::operator*();
  }
};

static_assert(pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<std::add_pointer_t>>());
static_assert(pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<ClassInvoker>>());
static_assert(pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<pro::detail::inline_meta_storage>>());
static_assert(pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<NonTrivialDestructionStorage>>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithoutInvoker>());
static_assert(
    !pro::detail::is_metadata_policy_well_formed<PolicyWithoutStorage>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<ThrowingDefaultConstructionInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<ThrowingConstructionInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<ThrowingCopyInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<ThrowingCopyAssignmentInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<NonTrivialDestructionInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<InvokerWithoutBoolConversion>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithInvoker<UncallableInvoker>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingCopyStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingDefaultConstructionStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingCopyAssignmentStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingDestructionStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingResetStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ThrowingEmplaceStorage>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<StorageWithoutBoolConversion>>());
static_assert(!pro::detail::is_metadata_policy_well_formed<
              PolicyWithStorage<ByValueStorage>>());

using M0 = SlotMeta<0>;
using M1 = SlotMeta<1>;
using M2 = SlotMeta<2>;
using M01 = pro::detail::proxy_meta_base_impl<M0, M1>;
using M02 = pro::detail::proxy_meta_base_impl<M0, M2>;
using M012 = pro::detail::proxy_meta_base_impl<M01, M2>;

static_assert(std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M0>>,
                             std::tuple<M0>>);
static_assert(std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M1>>,
                             std::tuple<M0, M1>>);
static_assert(std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M01>>,
                             std::tuple<M01>>);
static_assert(std::is_same_v<pro::detail::unique_types_t<std::tuple<M01, M0>>,
                             std::tuple<M01>>);
static_assert(
    std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M2, M01>>,
                   std::tuple<M01, M2>>);
static_assert(
    std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M01, M02>>,
                   std::tuple<M01, M02>>);
static_assert(
    std::is_same_v<pro::detail::unique_types_t<std::tuple<M0, M01, M012>>,
                   std::tuple<M012>>);

struct LeadDispatch {
  void operator()(auto&&) const noexcept {}
};
using LeadInvocationMeta =
    pro::detail::invocation_meta<pro::inline_metadata, true, LeadDispatch,
                                 void() noexcept>;
struct LeadFacade : pro::facade_builder::build {};
using LeadProxyMeta = pro::detail::proxy_meta<LeadFacade, pro::inline_metadata>;

static_assert(std::is_same_v<
              pro::detail::proxy_meta_base_t<>,
              pro::detail::proxy_meta_base_impl<pro::detail::sentinel_meta>>);
static_assert(
    std::is_same_v<
        pro::detail::proxy_meta_base_t<M0, M1>,
        pro::detail::proxy_meta_base_impl<pro::detail::sentinel_meta, M0, M1>>);
static_assert(std::is_same_v<pro::detail::proxy_meta_base_t<PlainMeta<0>>,
                             pro::detail::proxy_meta_base_impl<
                                 pro::detail::sentinel_meta, PlainMeta<0>>>);
static_assert(
    std::is_same_v<pro::detail::proxy_meta_base_t<LeadInvocationMeta, M0>,
                   pro::detail::proxy_meta_base_impl<LeadInvocationMeta, M0>>);
static_assert(
    std::is_same_v<pro::detail::proxy_meta_base_t<LeadProxyMeta, M0>,
                   pro::detail::proxy_meta_base_impl<LeadProxyMeta, M0>>);
static_assert(std::is_trivially_copyable_v<
                  pro::detail::proxy_meta_base_t<LeadInvocationMeta>> ==
              std::is_trivially_copyable_v<LeadInvocationMeta>);
static_assert(!std::is_trivially_copyable_v<M01>);

static_assert(std::is_nothrow_convertible_v<const M01&, const M0&>);
static_assert(std::is_nothrow_convertible_v<const M01&, const M1&>);
static_assert(!std::is_nothrow_convertible_v<const M01&, const M2&>);
static_assert(!std::is_nothrow_convertible_v<const M0&, const M01&>);

inline constexpr pro::detail::proxy_meta_base_impl<M1, M0> kReordered{
    std::in_place_type<int>};
inline constexpr pro::detail::proxy_meta_base_impl<M01, M02> kDiamond{
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
