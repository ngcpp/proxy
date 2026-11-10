// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include "utils.h"
#include <gtest/gtest.h>
#include <memory>
#include <proxy/proxy.h>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace proxy_reflection_tests_detail {

struct TraitsReflector {
public:
  TraitsReflector() = default;
  template <class T>
  constexpr explicit TraitsReflector(std::in_place_type_t<T>) noexcept
      : is_default_constructible_(std::is_default_constructible_v<T>),
        is_copy_constructible_(std::is_copy_constructible_v<T>),
        is_nothrow_move_constructible_(std::is_nothrow_move_constructible_v<T>),
        is_nothrow_destructible_(std::is_nothrow_destructible_v<T>),
        is_trivial_(std::is_trivially_default_constructible_v<T> &&
                    std::is_trivially_copyable_v<T>) {}

  template <class Self, class R>
  struct accessor {
    const TraitsReflector& ReflectTraits() const noexcept {
      return reflect<R>(static_cast<const Self&>(*this));
    }
  };

  bool is_default_constructible_;
  bool is_copy_constructible_;
  bool is_nothrow_move_constructible_;
  bool is_nothrow_destructible_;
  bool is_trivial_;
};

struct TestRttiFacade : pro::facade_builder                           //
                        ::add_reflection<utils::RttiReflector>        //
                        ::add_direct_reflection<utils::RttiReflector> //
                        ::build {};

struct TestTraitsFacade : pro::facade_builder                      //
                          ::add_direct_reflection<TraitsReflector> //
                          ::build {};

struct DescriptionAccess {
  template <class Self, class... Ds>
  struct PRO5D_ENFORCE_EBO accessor : accessor<Self, Ds>... {};
  template <class Self, class D>
  struct accessor<Self, pro::proxy_operation<D, std::string() const>> {
    std::string Describe() const {
      return invoke<D, std::string() const>(static_cast<const Self&>(*this));
    }
  };
  template <class Self, class M>
  struct accessor<Self, pro::proxy_reflection<M>> {
    const char* GetTypeName() const noexcept {
      return reflect<M>(static_cast<const Self&>(*this)).info->name();
    }
  };
};

struct DescribeDispatch {
  template <class T>
  std::string operator()(const T& self) const {
    return std::to_string(self);
  }
};

struct TypeInfoReflector {
  TypeInfoReflector() = default;
  template <class T>
  constexpr explicit TypeInfoReflector(std::in_place_type_t<T>) noexcept
      : info(&typeid(T)) {}

  const std::type_info* info;
};

struct TestDescriptionFacade {
  struct DescribeConvention {
    static constexpr bool is_direct = false;
    using dispatch_type = DescribeDispatch;
    using overload_type = std::string() const;
    using access_type = DescriptionAccess;
  };
  struct TypeInfoReflection {
    static constexpr bool is_direct = false;
    using reflector_type = TypeInfoReflector;
    using access_type = DescriptionAccess;
  };
  using super_types = std::tuple<>;
  using convention_types = std::tuple<DescribeConvention>;
  using reflection_types = std::tuple<TypeInfoReflection>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};

} // namespace proxy_reflection_tests_detail

namespace detail = proxy_reflection_tests_detail;

TEST(ProxyReflectionTests, TestRtti_RawPtr) {
  int foo = 123;
  pro::proxy<detail::TestRttiFacade> p = &foo;
  ASSERT_STREQ(p.GetTypeName(), typeid(int*).name());
  ASSERT_STREQ(p->GetTypeName(), typeid(int).name());
}

TEST(ProxyReflectionTests, TestRtti_FancyPtr) {
  pro::proxy<detail::TestRttiFacade> p = std::make_unique<double>(1.23);
  ASSERT_STREQ(p.GetTypeName(), typeid(std::unique_ptr<double>).name());
  ASSERT_STREQ(p->GetTypeName(), typeid(double).name());
}

TEST(ProxyReflectionTests, TestTraits_RawPtr) {
  int foo = 123;
  pro::proxy<detail::TestTraitsFacade> p = &foo;
  ASSERT_EQ(p.ReflectTraits().is_default_constructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_copy_constructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_nothrow_move_constructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_nothrow_destructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_trivial_, true);
}

TEST(ProxyReflectionTests, TestTraits_FancyPtr) {
  pro::proxy<detail::TestTraitsFacade> p = std::make_unique<double>(1.23);
  ASSERT_EQ(p.ReflectTraits().is_default_constructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_copy_constructible_, false);
  ASSERT_EQ(p.ReflectTraits().is_nothrow_move_constructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_nothrow_destructible_, true);
  ASSERT_EQ(p.ReflectTraits().is_trivial_, false);
}

TEST(ProxyReflectionTests, TestSharedAccess) {
  using Self = pro::proxy_indirect_accessor<detail::TestDescriptionFacade>;
  static_assert(
      std::is_base_of_v<detail::DescriptionAccess::accessor<
                            Self,
                            pro::proxy_operation<detail::DescribeDispatch,
                                                 std::string() const>,
                            pro::proxy_reflection<detail::TypeInfoReflector>>,
                        Self>);
  static_assert(sizeof(pro::proxy<detail::TestDescriptionFacade>) ==
                3 * sizeof(void*));
  static_assert(sizeof(pro::proxy_view<detail::TestDescriptionFacade>) ==
                2 * sizeof(void*));
  pro::proxy<detail::TestDescriptionFacade> p =
      pro::make_proxy<detail::TestDescriptionFacade>(123);
  ASSERT_EQ(p->Describe(), "123");
  ASSERT_STREQ(p->GetTypeName(), typeid(int).name());
  double value = 1.5;
  pro::proxy_view<detail::TestDescriptionFacade> pv =
      pro::make_proxy_view<detail::TestDescriptionFacade>(value);
  ASSERT_EQ(pv->Describe(), std::to_string(value));
  ASSERT_STREQ(pv->GetTypeName(), typeid(double).name());
}
