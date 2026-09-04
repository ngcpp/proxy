// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include "utils.h"
#include <gtest/gtest.h>
#include <proxy/proxy.h>

namespace proxy_lifetime_tests_detail {

struct TestFacade
    : pro::facade_builder                                        //
      ::add_convention<utils::spec::FreeToString, std::string()> //
      ::support_relocation<pro::constraint_level::nontrivial>    //
      ::support_copy<pro::constraint_level::nontrivial>          //
      ::add_direct_convention<pro::conversion_dispatch,
                              utils::LifetimeTracker::Session() const&,
                              utils::LifetimeTracker::Session() &&> //
      ::build {};

struct TestTrivialFacade
    : pro::facade_builder                                   //
      ::add_facade<utils::spec::Stringable>                 //
      ::support_copy<pro::constraint_level::trivial>        //
      ::support_relocation<pro::constraint_level::trivial>  //
      ::support_destruction<pro::constraint_level::trivial> //
      ::build {};

struct TestRttiFacade : pro::facade_builder                           //
                        ::add_direct_reflection<utils::RttiReflector> //
                        ::add_facade_with_substitution<TestFacade>    //
                        ::build {};

// Additional static asserts for super conversion
static_assert(std::is_convertible_v<pro::proxy<TestTrivialFacade>,
                                    pro::proxy<utils::spec::Stringable>>);
static_assert(
    std::is_convertible_v<pro::proxy<TestRttiFacade>, pro::proxy<TestFacade>>);
static_assert(!std::is_convertible_v<pro::proxy<utils::spec::Stringable>,
                                     pro::proxy<TestTrivialFacade>>);
static_assert(!std::is_convertible_v<pro::proxy<TestTrivialFacade>,
                                     pro::proxy<TestFacade>>);

} // namespace proxy_lifetime_tests_detail

namespace detail = proxy_lifetime_tests_detail;

TEST(ProxyLifetimeTests, TestDefaultConstrction) {
  pro::proxy<detail::TestFacade> p;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullConstrction_Nullptr) {
  pro::proxy<detail::TestFacade> p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullConstrction_TypedNullPointer) {
  int* v = nullptr;
  pro::proxy<utils::spec::Stringable> p = v;
  ASSERT_TRUE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p =
        utils::LifetimeTracker::Session(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_FromValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session another_session{&tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<detail::TestFacade> p = another_session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlace) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlace_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<detail::TestFacade> p{
          std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlaceInitializerList) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>,
        {1, 2, 3},
        &tracker};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyConstrction_InPlaceInitializerList_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      pro::proxy<detail::TestFacade> p{
          std::in_place_type<utils::LifetimeTracker::Session>,
          {1, 2, 3},
          &tracker};
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_,
                utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto p2 = p1;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyConstrction_FromNull) {
  pro::proxy<detail::TestFacade> p1;
  auto p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromValue_Trivial) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session session{&tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestTrivialFacade> p1 = &session;
    ASSERT_TRUE(p1.has_value());
    auto p2 = std::move(p1);
    ASSERT_TRUE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveConstrction_FromNull) {
  pro::proxy<detail::TestFacade> p1;
  auto p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestNullAssignment_FromNullptr_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p = nullptr;
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestNullAssignment_FromTypedNullPointer_ToValue) {
  pro::proxy<utils::spec::Stringable> p =
      pro::make_proxy<utils::spec::Stringable>(123);
  p = std::shared_ptr<int>{};
  ASSERT_TRUE(p.has_value());
}

TEST(ProxyLifetimeTests, TestNullAssignment_ToNull) {
  pro::proxy<detail::TestFacade> p;
  p = nullptr;
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p = utils::LifetimeTracker::Session{&tracker};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 4");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    utils::LifetimeTracker::Session session{&tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    p = utils::LifetimeTracker::Session{&tracker};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 3");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    utils::LifetimeTracker::Session session{&tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p = session;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>(&tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlace_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>(&tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kValueConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p.emplace<utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 2");
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(
        2, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests,
     TestPolyAssignment_InPlaceInitializerList_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_,
                utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestPolyAssignment_InPlaceInitializerList_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    p.emplace<utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(
        1, utils::LifetimeOperationType::kInitializerListConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests,
     TestPolyAssignment_InPlaceInitializerList_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p.emplace<utils::LifetimeTracker::Session>({1, 2, 3}, &tracker);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_,
                utils::LifetimeOperationType::kInitializerListConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToSelf) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif // __clang__
    p = p;
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1;
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p1 = p2;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 3");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1;
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = p2;
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kCopyConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 1");
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2;
    p1 = p2;
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToSelf) {
  pro::proxy<detail::TestFacade> p;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif // __clang__
  p = p;
#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestCopyAssignment_FromNull_ToNull) {
  pro::proxy<detail::TestFacade> p1;
  pro::proxy<detail::TestFacade> p2;
  p1 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 3");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToValue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = std::move(p2);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToSelf) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif // __clang__
    p = std::move(p);
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif // __clang__
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToNull) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1;
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p1 = std::move(p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromValue_ToNull_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1;
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      p1 = std::move(p2);
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p1.has_value());
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2;
    p1 = std::move(p2);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToSelf) {
  pro::proxy<detail::TestFacade> p;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif // __clang__
  p = std::move(p);
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic pop
#endif // __clang__
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestMoveAssignment_FromNull_ToNull) {
  pro::proxy<detail::TestFacade> p1;
  pro::proxy<detail::TestFacade> p2;
  p1 = std::move(p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestHasValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    ASSERT_FALSE(p.has_value());
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestOperatorBool) {
  // Implicit conversion to bool shall be prohibited.
  static_assert(
      !std::is_nothrow_convertible_v<pro::proxy<detail::TestFacade>, bool>);

  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    ASSERT_FALSE(static_cast<bool>(p));
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(static_cast<bool>(p));
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestEqualsToNullptr) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p;
    ASSERT_TRUE(p == nullptr);
    ASSERT_TRUE(nullptr == p);
    p.emplace<utils::LifetimeTracker::Session>(&tracker);
    ASSERT_TRUE(p != nullptr);
    ASSERT_TRUE(nullptr != p);
    ASSERT_EQ(ToString(*p), "Session 1");
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    p.reset();
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestReset_FromNull) {
  pro::proxy<detail::TestFacade> p;
  p.reset();
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Value_Value) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 4");
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 5");
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(4,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(5,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(5, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(4, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Value_Self) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    swap(p, p);
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 3");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    expected_ops.emplace_back(3,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(3, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Value_Null) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2;
    swap(p1, p2);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Null_Value) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p1;
    pro::proxy<detail::TestFacade> p2{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    swap(p1, p2);
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 2");
    ASSERT_FALSE(p2.has_value());
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, TestSwap_Null_Self) {
  pro::proxy<detail::TestFacade> p;
  swap(p, p);
  ASSERT_FALSE(p.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Null_Null) {
  pro::proxy<detail::TestFacade> p1;
  pro::proxy<detail::TestFacade> p2;
  swap(p1, p2);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, TestSwap_Trivial) {
  pro::proxy<detail::TestTrivialFacade> p1 =
      pro::make_proxy<detail::TestTrivialFacade>(123);
  pro::proxy<detail::TestTrivialFacade> p2 =
      pro::make_proxy<detail::TestTrivialFacade>(456);
  swap(p1, p2);
  ASSERT_TRUE(p1.has_value());
  ASSERT_EQ(ToString(*p1), "456");
  ASSERT_TRUE(p2.has_value());
  ASSERT_EQ(ToString(*p2), "123");
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Lvalue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto session = utils::LifetimeTracker::Session{p};
    ASSERT_TRUE(p.has_value());
    ASSERT_EQ(ToString(*p), "Session 1");
    ASSERT_EQ(to_string(session), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Rvalue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    auto session = utils::LifetimeTracker::Session{std::move(p)};
    ASSERT_FALSE(p.has_value());
    ASSERT_EQ(to_string(session), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_DirectConvension_Rvalue_Exception) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestFacade> p{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    tracker.ThrowOnNextConstruction();
    bool exception_thrown = false;
    try {
      auto session = static_cast<utils::LifetimeTracker::Session>(std::move(p));
    } catch (const utils::ConstructionFailure& e) {
      exception_thrown = true;
      ASSERT_EQ(e.type_, utils::LifetimeOperationType::kMoveConstruction);
    }
    ASSERT_TRUE(exception_thrown);
    ASSERT_FALSE(p.has_value());
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_CopySubstitution_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestRttiFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2 = p1;
    ASSERT_TRUE(p1.has_value());
    ASSERT_EQ(ToString(*p1), "Session 1");
    ASSERT_STREQ(p1.GetTypeName(),
                 typeid(utils::LifetimeTracker::Session).name());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kCopyConstruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_CopySubstitution_FromNull) {
  pro::proxy<detail::TestRttiFacade> p1;
  pro::proxy<detail::TestFacade> p2 = p1;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, Test_MoveSubstitution_FromValue) {
  utils::LifetimeTracker tracker;
  std::vector<utils::LifetimeOperation> expected_ops;
  {
    pro::proxy<detail::TestRttiFacade> p1{
        std::in_place_type<utils::LifetimeTracker::Session>, &tracker};
    expected_ops.emplace_back(1,
                              utils::LifetimeOperationType::kValueConstruction);
    pro::proxy<detail::TestFacade> p2 = std::move(p1);
    ASSERT_FALSE(p1.has_value());
    ASSERT_TRUE(p2.has_value());
    ASSERT_EQ(ToString(*p2), "Session 2");
    expected_ops.emplace_back(2,
                              utils::LifetimeOperationType::kMoveConstruction);
    expected_ops.emplace_back(1, utils::LifetimeOperationType::kDestruction);
    ASSERT_TRUE(tracker.GetOperations() == expected_ops);
  }
  expected_ops.emplace_back(2, utils::LifetimeOperationType::kDestruction);
  ASSERT_TRUE(tracker.GetOperations() == expected_ops);
}

TEST(ProxyLifetimeTests, Test_MoveSubstitution_FromNull) {
  pro::proxy<detail::TestRttiFacade> p1;
  pro::proxy<detail::TestFacade> p2 = std::move(p1);
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyLifetimeTests, Test_MoveSubstitution_Trivial) {
  // A trivially copyable proxy has no move constructor; a conversion from an
  // rvalue shall fall back to the copy constructor and leave rhs intact, just
  // like a move between two proxies of the same facade.
  struct Derived : pro::facade_builder                     //
                   ::add_facade<detail::TestTrivialFacade> //
                   ::build {};
  int v = 123;
  pro::proxy<Derived> p1 = &v;
  pro::proxy<detail::TestTrivialFacade> p2 = std::move(p1);
  ASSERT_TRUE(p1.has_value());
  ASSERT_EQ(ToString(*p1), "123");
  ASSERT_EQ(ToString(*p2), "123");
}

TEST(ProxyLifetimeTests, Test_CopyAssignment_NoRelocation) {
  // A facade that forbids relocation cannot stage a throwing assignment in a
  // temporary, because committing the temporary would need a move assignment.
  // Assignment shall still work, emptying *this up front instead.
  struct Pinned : pro::facade_builder //
                  ::add_convention<utils::spec::FreeToString,
                                   std::string() const>             //
                  ::support_copy<pro::constraint_level::nontrivial> //
                  ::support_relocation<pro::constraint_level::none> //
                  ::build {};
  struct PinnedDerived : pro::facade_builder  //
                         ::add_facade<Pinned> //
                         ::build {};
  int v1 = 111, v2 = 222;
  pro::proxy<Pinned> p1{utils::ThrowingCopyPtr<int>{&v1}};
  pro::proxy<Pinned> p2{utils::ThrowingCopyPtr<int>{&v2}};
  p1 = p2; // Same-facade copy assignment
  ASSERT_EQ(ToString(*p1), "222");
  ASSERT_EQ(ToString(*p2), "222");

  pro::proxy<PinnedDerived> p3{utils::ThrowingCopyPtr<int>{&v1}};
  p1 = p3; // Converting copy assignment
  ASSERT_EQ(ToString(*p1), "111");
  ASSERT_TRUE(p3.has_value());

  p1 = utils::ThrowingCopyPtr<int>{&v2}; // Pointer assignment
  ASSERT_EQ(ToString(*p1), "222");
}

TEST(ProxyLifetimeTests, Test_PointerAssignment_NoRelocationNoCopy) {
  // Same shape, but the facade supports neither relocation nor copy, so there
  // is no assignment operator a temporary could be committed with at all.
  struct Pinned : pro::facade_builder //
                  ::add_convention<utils::spec::FreeToString,
                                   std::string() const>             //
                  ::support_relocation<pro::constraint_level::none> //
                  ::build {};
  int v1 = 111, v2 = 222;
  pro::proxy<Pinned> p{utils::ThrowingCopyPtr<int>{&v1}};
  p = utils::ThrowingCopyPtr<int>{&v2};
  ASSERT_EQ(ToString(*p), "222");
}

TEST(ProxyLifetimeTests, Test_PointerAssignment_ThrowingInitialization) {
  // When the initialization can throw, it is staged in a temporary that the
  // move assignment commits, so a failure leaves *this unchanged.
  struct Movable : pro::facade_builder //
                   ::add_convention<utils::spec::FreeToString,
                                    std::string() const>                   //
                   ::support_copy<pro::constraint_level::nontrivial>       //
                   ::support_relocation<pro::constraint_level::nontrivial> //
                   ::build {};
  // A trivially copyable proxy has no move assignment, and this one forbids
  // relocation outright, but its trivial copy assignment commits the temporary
  // just as well.
  struct TriviallyCopyable
      : pro::facade_builder //
        ::add_convention<utils::spec::FreeToString,
                         std::string() const>             //
        ::support_copy<pro::constraint_level::trivial>    //
        ::support_relocation<pro::constraint_level::none> //
        ::build {};
  int v1 = 111, v2 = 222;

  pro::proxy<Movable> p1{std::in_place_type<utils::ThrowOnMovePtr<int>>, &v1};
  ASSERT_THROW(p1 = utils::ThrowOnMovePtr<int>{&v2},
               utils::ConstructionFailure);
  ASSERT_TRUE(p1.has_value());
  ASSERT_EQ(ToString(*p1), "111");

  pro::proxy<TriviallyCopyable> p2{
      std::in_place_type<utils::ThrowOnMovePtr<int>>, &v1};
  ASSERT_THROW(p2 = utils::ThrowOnMovePtr<int>{&v2},
               utils::ConstructionFailure);
  ASSERT_TRUE(p2.has_value());
  ASSERT_EQ(ToString(*p2), "111");
}
