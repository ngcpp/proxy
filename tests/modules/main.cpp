#include <gtest/gtest.h>
#include <iostream>
#include <utility>

import proxy.v5;
import foo;
import foo_impl;

auto user(pro::proxy<Foo> p) { return p->GetFoo(); }

TEST(ProxyModuleSupportTests, TestBasic) {
  MyFoo foo;
  ASSERT_EQ(user(&foo), 42);
}

TEST(ProxyModuleSupportTests, TestRepeatedSpecialization) {
  MyFoo foo;
  pro::proxy<Foo> p = &foo;
  ASSERT_EQ(user(std::move(p)), 42);
}
