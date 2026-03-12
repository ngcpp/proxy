// Copyright (c) 2022-2026 Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(                                                               \
    disable : 4702) // False alarm from MSVC: warning C4702: unreachable code
#endif              // defined(_MSC_VER) && !defined(__clang__)
#include <proxy/proxy.h>
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif // defined(_MSC_VER) && !defined(__clang__)
#include <vector>

namespace proxy_regression_tests_details {

template <class It, class F>
bool operator==(const It& it, const pro::proxy<F>& rhs) noexcept
  requires(!std::is_same_v<It, pro::proxy<F>>)
{
  return typeid(It) == proxy_typeid(rhs) && it == proxy_cast<const It&>(rhs);
}

template <class F>
using SelfComparisonOverload = bool(const pro::proxy<F>& rhs) const noexcept;

template <class T>
struct Iterator
    : pro::facade_builder                                                    //
      ::add_skill<pro::skills::direct_rtti>                                  //
      ::support_relocation<pro::constraint_level::nothrow>                   //
      ::restrict_layout<4 * sizeof(void*)>                                   //
      ::add_direct_convention<pro::operator_dispatch<"++">, void() noexcept> //
      ::add_direct_convention<
          pro::operator_dispatch<"!=">,
          pro::facade_aware_overload_t<SelfComparisonOverload>> //
      ::add_convention<pro::implicit_conversion_dispatch,
                       T&() const noexcept> //
      ::build {};

PRO_DEF_MEM_DISPATCH(MemBegin, begin);
PRO_DEF_MEM_DISPATCH(MemEnd, end);
template <class T>
struct Range : pro::facade_builder                                          //
               ::add_convention<MemBegin, pro::proxy<Iterator<T>>()>        //
               ::template add_convention<MemEnd, pro::proxy<Iterator<T>>()> //
               ::build {};

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

} // namespace proxy_regression_tests_details

namespace details = proxy_regression_tests_details;

// https://github.com/microsoft/proxy/issues/213
TEST(ProxyRegressionTests, TestUnexpectedCompilerWarning) {
  struct MyTrivialFacade
      : pro::facade_builder                                                  //
        ::add_convention<pro::operator_dispatch<"()">, void(), void() const> //
        ::support_copy<pro::constraint_level::trivial>                       //
        ::support_relocation<pro::constraint_level::trivial>                 //
        ::support_destruction<pro::constraint_level::trivial>                //
        ::build {};
  int side_effect = 0;
  pro::proxy<MyTrivialFacade> p =
      pro::make_proxy<MyTrivialFacade>([&] { side_effect = 1; });
  (*p)();
  EXPECT_EQ(side_effect, 1);
}

// https://github.com/microsoft/proxy/issues/254
TEST(ProxyRegressionTests, TestProxiableSelfDependency) {
  std::vector<int> original{1, 2, 123};
  std::vector<int> expected;
  pro::proxy<details::Range<int>> p = &original;
  for (int i : *p) {
    expected.push_back(i);
  }
  EXPECT_EQ(expected, original);
}

// https://github.com/ngcpp/proxy/issues/10
TEST(ProxyRegressionTests, TestWeakDispatchReferenceReturningOverload) {
  struct MyFacade
      : pro::facade_builder                                           //
        ::add_convention<pro::weak_dispatch<details::MemFun>, int&()> //
        ::build {};
  static_assert(pro::proxiable<int*, MyFacade>);

  int v = 123;
  pro::proxy<MyFacade> p = &v;
  EXPECT_THROW(p->Fun(), pro::not_implemented);
}
