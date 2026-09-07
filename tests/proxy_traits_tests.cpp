// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include "utils.h"
#include <array>
#include <proxy/proxy.h>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace proxy_traits_tests_detail {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial,
          std::size_t kSize, std::size_t kAlignment>
struct MockPtr {
  using element_type = MockPtr;

  MockPtr() = default;
  MockPtr(int) noexcept {}
  MockPtr(const MockPtr&)
    requires(kCopyable && !kTrivial)
  {}
  MockPtr(const MockPtr&) noexcept
    requires(kTrivial)
  = default;
  MockPtr(MockPtr&&) noexcept
    requires(kNothrowRelocatable && !kTrivial)
  {}
  MockPtr(MockPtr&&) noexcept
    requires(kTrivial)
  = default;
  const MockPtr* operator->() const noexcept { return this; }

  alignas(kAlignment) char dummy_[kSize];
};
using MockMovablePtr =
    MockPtr<true, false, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyablePtr =
    MockPtr<true, true, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyableSmallPtr =
    MockPtr<true, true, false, sizeof(void*), alignof(void*)>;
using MockTrivialPtr = MockPtr<true, true, true, sizeof(void*), alignof(void*)>;
using MockFunctionPtr = void (*)();

} // namespace proxy_traits_tests_detail

namespace pro {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial,
          std::size_t kSize, std::size_t kAlignment>
struct is_bitwise_trivially_relocatable<proxy_traits_tests_detail::MockPtr<
    kNothrowRelocatable, kCopyable, kTrivial, kSize, kAlignment>>
    : std::true_type {};

} // namespace pro

namespace proxy_traits_tests_detail {

struct DefaultFacade : pro::facade_builder::build {};
static_assert(
    std::is_same_v<pro::proxy<DefaultFacade>::facade_type, DefaultFacade>);
static_assert(DefaultFacade::copyability == pro::constraint_level::none);
static_assert(DefaultFacade::relocatability == pro::constraint_level::trivial);
static_assert(DefaultFacade::destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::max_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::max_align >= sizeof(void*));
static_assert(std::is_same_v<DefaultFacade::convention_types, std::tuple<>>);
static_assert(std::is_same_v<DefaultFacade::reflection_types, std::tuple<>>);
static_assert(
    std::is_nothrow_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    !std::is_trivially_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                    std::in_place_type_t<MockMovablePtr>, int>);
static_assert(
    std::is_nothrow_constructible_v<
        pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<
              pro::proxy<DefaultFacade>,
              std::in_place_type_t<MockCopyableSmallPtr>, int>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                    std::in_place_type_t<MockTrivialPtr>, int>);
static_assert(std::is_nothrow_constructible_v<
              pro::proxy<DefaultFacade>, std::in_place_type_t<MockFunctionPtr>,
              MockFunctionPtr>);
static_assert(sizeof(pro::proxy<DefaultFacade>) == 3 * sizeof(void*));

static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    !std::is_trivially_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(pro::proxiable<MockMovablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, DefaultFacade>);
static_assert(pro::proxiable<MockTrivialPtr, DefaultFacade>);
static_assert(pro::proxiable<MockFunctionPtr, DefaultFacade>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockCopyablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>,
                                           MockCopyableSmallPtr>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockFunctionPtr>);

struct CopyableFacade : pro::facade_builder                               //
                        ::support_copy<pro::constraint_level::nontrivial> //
                        ::build {};
static_assert(std::is_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(
    !std::is_nothrow_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(
    !std::is_trivially_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableFacade>);
static_assert(pro::proxiable<MockFunctionPtr, CopyableFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_assignable_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockCopyablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>,
                                           MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableFacade>) == 3 * sizeof(void*));

struct CopyableSmallFacade
    : pro::facade_builder                               //
      ::add_skill<pro::skills::slim>                    //
      ::support_copy<pro::constraint_level::nontrivial> //
      ::build {};
static_assert(!pro::proxiable<MockMovablePtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockFunctionPtr, CopyableSmallFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableSmallFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableSmallFacade>,
                                           MockCopyableSmallPtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableSmallFacade>) == 2 * sizeof(void*));

struct TrivialFacade : pro::facade_builder                                   //
                       ::restrict_layout<sizeof(void*), alignof(void*)>      //
                       ::support_copy<pro::constraint_level::trivial>        //
                       ::support_relocation<pro::constraint_level::trivial>  //
                       ::support_destruction<pro::constraint_level::trivial> //
                       ::build {};
#ifdef PRO4D_HAS_PAC
static_assert(std::is_nothrow_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
#else
static_assert(
    std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(
    std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
#endif // PRO4D_HAS_PAC
static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyableSmallPtr, TrivialFacade>);
static_assert(pro::proxiable<MockTrivialPtr, TrivialFacade>);
static_assert(pro::proxiable<MockFunctionPtr, TrivialFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(std::is_assignable_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<TrivialFacade>) == 2 * sizeof(void*));

struct ReflectionOfSmallPtr {
  ReflectionOfSmallPtr() = default;
  template <class P>
    requires(sizeof(P) <= sizeof(void*))
  constexpr ReflectionOfSmallPtr(std::in_place_type_t<P>) noexcept {}
};
struct RelocatableFacadeWithReflection
    : pro::facade_builder                           //
      ::add_direct_reflection<ReflectionOfSmallPtr> //
      ::build {};
static_assert(!pro::proxiable<MockMovablePtr, RelocatableFacadeWithReflection>);
static_assert(
    !pro::proxiable<MockCopyablePtr, RelocatableFacadeWithReflection>);
static_assert(
    pro::proxiable<MockCopyableSmallPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockFunctionPtr, RelocatableFacadeWithReflection>);

struct ThrowingReflection {
  ThrowingReflection() = default;
  template <class P>
  explicit ThrowingReflection(std::in_place_type_t<P>) {
    throw std::runtime_error{"Not supported"};
  }
};
struct FacadeWithThrowingReflection : pro::facade_builder                  //
                                      ::add_reflection<ThrowingReflection> //
                                      ::build {};
static_assert(!pro::proxiable<MockTrivialPtr, FacadeWithThrowingReflection>);

struct FacadeWithTupleLikeConventions {
  struct ToStringConvention {
    static constexpr bool is_direct = false;
    using dispatch_type = utils::spec::FreeToString;
    using overload_type = std::string();
  };
  using super_types = std::tuple<>;
  using convention_types = std::array<ToStringConvention, 1>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(pro::facade<FacadeWithTupleLikeConventions>);

struct BadFacade_MissingConventionTypes {
  using super_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_MissingConventionTypes>);

struct BadFacade_BadConventionTypes {
  using super_types = std::tuple<>;
  using convention_types = int;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadConventionTypes>);

struct FacadeWithSuperTypes {
  using super_types = std::tuple<FacadeWithTupleLikeConventions>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(pro::facade<FacadeWithSuperTypes>);

// A facade shall be no less conservative than any of its supers
struct BadFacade_SuperWithSmallerLayout {
  using super_types = std::tuple<FacadeWithTupleLikeConventions>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 4 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_SuperWithSmallerLayout>);

struct BadFacade_SuperWithStrongerConstraint {
  using super_types = std::tuple<FacadeWithTupleLikeConventions>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nontrivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_SuperWithStrongerConstraint>);

struct BadFacade_MissingSuperTypes {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_MissingSuperTypes>);

struct BadFacade_BadSuperTypes {
  using super_types = int;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadSuperTypes>);

// A super that is not itself a facade makes the whole facade ill-formed
struct BadFacade_BadSuperType {
  using super_types = std::tuple<BadFacade_BadConventionTypes>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadSuperType>);

struct BadFacade_MissingConstraints {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
};
static_assert(!pro::facade<BadFacade_MissingConstraints>);

struct BadFacade_BadConstraints_UnexpectedType {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = 0;
};
static_assert(!pro::facade<BadFacade_BadConstraints_UnexpectedType>);

// Well-formed facade_impl specialization
static_assert(pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad size (max_size should be positive)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 0, 4,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad size (max_size should be a multiple of max_align)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 10, 4,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad alignment (max_align should be a power of 2)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 6, 6,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad copyability (less than constraint_level::none)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  (pro::constraint_level)-1, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad copyability (greater than constraint_level::trivial)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  (pro::constraint_level)100, pro::constraint_level::trivial,
                  pro::constraint_level::nothrow>>);

// Bad relocatability (less than constraint_level::none)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  pro::constraint_level::none, (pro::constraint_level)-1,
                  pro::constraint_level::nothrow>>);

// Bad relocatability (greater than constraint_level::trivial)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  pro::constraint_level::none, (pro::constraint_level)100,
                  pro::constraint_level::nothrow>>);

// Bad destructibility (less than constraint_level::none)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  (pro::constraint_level)-1>>);

// Bad destructibility (greater than constraint_level::trivial)
static_assert(!pro::facade<pro::detail::facade_impl<
                  std::tuple<>, std::tuple<>, std::tuple<>, 8, 4,
                  pro::constraint_level::none, pro::constraint_level::trivial,
                  (pro::constraint_level)100>>);

struct BadFacade_BadConstraints_NotConstant {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static const std::size_t max_size;
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadConstraints_NotConstant>);
const std::size_t BadFacade_BadConstraints_NotConstant::max_size =
    2 * sizeof(void*);
struct BadFacade_MissingReflectionTypes {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_MissingReflectionTypes>);

struct BadReflection {
  BadReflection() = delete;
};
struct BadFacade_BadReflectionType {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<BadReflection>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadReflectionType>);

PRO_DEF_MEM_DISPATCH(MemFoo, Foo);
PRO_DEF_MEM_DISPATCH(MemBar, Bar);
struct BigFacade : pro::facade_builder                                //
                   ::add_convention<MemFoo, void(), void(int)>        //
                   ::add_convention<MemBar, void(), void(int)>        //
                   ::add_direct_convention<MemFoo, void(), void(int)> //
                   ::add_direct_convention<MemBar, void(), void(int)> //
                   ::build {};
static_assert(sizeof(pro::proxy<BigFacade>) ==
              3 * sizeof(void*)); // Accessors should not add paddings

// A facade-aware convention declared on a super is substituted against the
// deriving facade, so proxiable shall reflect the substituted overload rather
// than only the one the super was checked with.
namespace facade_aware_from_super {

struct Super;
struct Mid;
struct Derived;
struct Sibling;
template <class F>
struct ReturnTypeTraits;
template <>
struct ReturnTypeTraits<Super> : std::type_identity<int> {};
template <>
struct ReturnTypeTraits<Mid> : std::type_identity<int> {};
template <>
struct ReturnTypeTraits<Derived> : std::type_identity<std::string> {};
template <>
struct ReturnTypeTraits<Sibling> : std::type_identity<int> {};
template <class F>
using GetOverload = typename ReturnTypeTraits<F>::type() const;
struct GetDispatch {
  template <class T>
  int operator()(const T& self) const {
    return self.Get();
  }
};
struct Super
    : pro::facade_builder //
      ::add_convention<GetDispatch,
                       pro::facade_aware_overload_t<GetOverload>>::build {};
struct Mid : pro::facade_builder //
             ::add_facade<Super> //
             ::build {};
struct Derived : pro::facade_builder //
                 ::add_facade<Mid>   //
                 ::build {};
struct Sibling : pro::facade_builder //
                 ::add_facade<Mid>   //
                 ::build {};
struct Impl {
  int Get() const { return 0; }
};

// GetOverload<Super> is int() const, which Impl satisfies; GetOverload<Derived>
// is std::string() const, which it does not. The convention is carried across
// two levels of super, so the check reaches Derived through Mid.
static_assert(pro::proxiable<Impl*, Super>);
static_assert(pro::proxiable<Impl*, Mid>);
static_assert(!pro::proxiable<Impl*, Derived>);
// A facade that fails the check does not taint its siblings.
static_assert(pro::proxiable<Impl*, Sibling>);

} // namespace facade_aware_from_super

struct FacadeWithSizeOfNonPowerOfTwo : pro::facade_builder   //
                                       ::restrict_layout<6u> //
                                       ::build {};
static_assert(pro::facade<FacadeWithSizeOfNonPowerOfTwo>);
static_assert(FacadeWithSizeOfNonPowerOfTwo::max_size == 6u);
static_assert(FacadeWithSizeOfNonPowerOfTwo::max_align == 2u);

template <std::size_t Size, std::size_t Align>
concept IsFacadeBuilderWellFormedWithGivenLayout =
    requires { typename pro::facade_builder::restrict_layout<Size, Align>; };
static_assert(IsFacadeBuilderWellFormedWithGivenLayout<6u, 1u>);
static_assert(!IsFacadeBuilderWellFormedWithGivenLayout<6u, 3u>);
static_assert(!IsFacadeBuilderWellFormedWithGivenLayout<1u, 2u>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenCopyability =
    requires { typename pro::facade_builder::support_copy<CL>; };
static_assert(
    IsFacadeBuilderWellFormedWithGivenCopyability<pro::constraint_level::none>);
static_assert(
    !IsFacadeBuilderWellFormedWithGivenCopyability<(pro::constraint_level)-1>);
static_assert(
    !IsFacadeBuilderWellFormedWithGivenCopyability<(pro::constraint_level)100>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenRelocatability =
    requires { typename pro::facade_builder::support_relocation<CL>; };
static_assert(IsFacadeBuilderWellFormedWithGivenRelocatability<
              pro::constraint_level::none>);
static_assert(!IsFacadeBuilderWellFormedWithGivenRelocatability<
              (pro::constraint_level)-1>);
static_assert(!IsFacadeBuilderWellFormedWithGivenRelocatability<
              (pro::constraint_level)100>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenDestructibility =
    requires { typename pro::facade_builder::support_destruction<CL>; };
static_assert(IsFacadeBuilderWellFormedWithGivenDestructibility<
              pro::constraint_level::none>);
static_assert(!IsFacadeBuilderWellFormedWithGivenDestructibility<
              (pro::constraint_level)-1>);
static_assert(!IsFacadeBuilderWellFormedWithGivenDestructibility<
              (pro::constraint_level)100>);

static_assert(!std::is_default_constructible_v<
              pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_copy_constructible_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_move_constructible_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_copy_assignable_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_move_assignable_v<pro::proxy_indirect_accessor<DefaultFacade>>);

// proxy shall not be constructible from an arbitrary class template
// instantiation. See https://github.com/microsoft/proxy/issues/366
template <class T>
struct ProxyWrapperTemplate {
  explicit ProxyWrapperTemplate(pro::proxy<DefaultFacade> p)
      : p_(std::move(p)) {}

  pro::proxy<DefaultFacade> p_;
};
static_assert(!pro::proxiable<int, DefaultFacade>);
static_assert(!std::is_constructible_v<pro::proxy<DefaultFacade>,
                                       ProxyWrapperTemplate<void>>);
static_assert(std::is_move_constructible_v<ProxyWrapperTemplate<void>>);

// proxiable shall not reject a specialization of proxy.
static_assert(pro::proxiable<pro::proxy_view<DefaultFacade>, DefaultFacade>);

} // namespace proxy_traits_tests_detail
