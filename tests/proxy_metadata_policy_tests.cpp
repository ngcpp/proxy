// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <cstddef>
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>

#include <proxy/proxy.h>

namespace proxy_metadata_policy_tests_detail {

template <std::size_t N>
using Tag = std::integral_constant<std::size_t, N>;

struct TestContext {
  template <class P>
  friend std::size_t invoke(TestContext ctx, std::size_t arg) noexcept {
    return ctx.value * P::value + arg;
  }

  std::size_t value;
};

using Invoker = pro::detail::invoker<TestContext, std::size_t(std::size_t)>;
using NoexceptInvoker =
    pro::detail::invoker<TestContext, std::size_t(std::size_t) noexcept>;

struct SmallMeta {
  constexpr SmallMeta() noexcept : value(0) {}
  template <class P>
  constexpr explicit SmallMeta(std::in_place_type_t<P>) noexcept
      : value(P::value) {}
  explicit operator bool() const noexcept { return value != 0; }

  std::size_t value;
};

struct LargeMeta : SmallMeta {
  constexpr LargeMeta() noexcept : SmallMeta(), extra(0) {}
  template <class P>
  constexpr explicit LargeMeta(std::in_place_type_t<P>) noexcept
      : SmallMeta(std::in_place_type<P>), extra(P::value * 10) {}

  std::size_t extra;
};

static_assert(sizeof(SmallMeta) <= sizeof(void*));
static_assert(sizeof(LargeMeta) > sizeof(void*));

using CompositeMeta = pro::detail::proxy_meta_base_t<Invoker, SmallMeta>;

static_assert(
    pro::detail::is_metadata_policy_well_formed<pro::compact_metadata>());
static_assert(
    pro::detail::is_metadata_policy_well_formed<pro::inline_metadata>());

static_assert(
    std::is_same_v<
        pro::compact_metadata::invoker<TestContext, std::size_t(std::size_t)>,
        Invoker>);
static_assert(
    std::is_same_v<
        pro::inline_metadata::invoker<TestContext, std::size_t(std::size_t)>,
        Invoker>);

static_assert(std::is_same_v<pro::compact_metadata::storage<SmallMeta>,
                             pro::detail::inline_meta_storage<SmallMeta>>);
static_assert(std::is_same_v<pro::compact_metadata::storage<LargeMeta>,
                             pro::detail::static_meta_storage<LargeMeta>>);
static_assert(std::is_same_v<pro::inline_metadata::storage<SmallMeta>,
                             pro::detail::inline_meta_storage<SmallMeta>>);
static_assert(std::is_same_v<pro::inline_metadata::storage<LargeMeta>,
                             pro::detail::inline_meta_storage<LargeMeta>>);

} // namespace proxy_metadata_policy_tests_detail

namespace detail = proxy_metadata_policy_tests_detail;

TEST(ProxyMetadataPolicyTests, TestInvoker_Null) {
  detail::Invoker inv{};
  ASSERT_FALSE(inv);
  detail::Invoker copy = inv;
  ASSERT_FALSE(copy);
}

TEST(ProxyMetadataPolicyTests, TestInvoker_Invoke) {
  detail::Invoker inv{std::in_place_type<detail::Tag<2>>};
  ASSERT_TRUE(inv);
  ASSERT_EQ(inv(detail::TestContext{3}, 4), 10u);
  detail::NoexceptInvoker noexcept_inv{std::in_place_type<detail::Tag<5>>};
  ASSERT_EQ(noexcept_inv(detail::TestContext{3}, 4), 19u);
}

TEST(ProxyMetadataPolicyTests, TestInvoker_CopyAndAssign) {
  detail::Invoker inv1{std::in_place_type<detail::Tag<2>>};
  detail::Invoker inv2{std::in_place_type<detail::Tag<5>>};
  detail::Invoker inv3 = inv1;
  ASSERT_EQ(inv3(detail::TestContext{3}, 4), 10u);
  inv3 = inv2;
  ASSERT_EQ(inv3(detail::TestContext{3}, 4), 19u);
  inv3 = detail::Invoker{};
  ASSERT_FALSE(inv3);
}

TEST(ProxyMetadataPolicyTests, TestStaticMetaStorage_Null) {
  pro::detail::static_meta_storage<detail::LargeMeta> s{};
  ASSERT_FALSE(s);
  auto copy = s;
  ASSERT_FALSE(copy);
}

TEST(ProxyMetadataPolicyTests, TestStaticMetaStorage_SharesMetadata) {
  pro::detail::static_meta_storage<detail::LargeMeta> s1{
      std::in_place_type<detail::Tag<1>>};
  pro::detail::static_meta_storage<detail::LargeMeta> s2{
      std::in_place_type<detail::Tag<1>>};
  pro::detail::static_meta_storage<detail::LargeMeta> s3{
      std::in_place_type<detail::Tag<2>>};
  ASSERT_TRUE(s1);
  ASSERT_EQ((*s1).value, 1u);
  ASSERT_EQ((*s1).extra, 10u);
  ASSERT_EQ(&*s1, &*s2);
  ASSERT_NE(&*s1, &*s3);
  auto copy = s3;
  ASSERT_EQ(&*copy, &*s3);
}

TEST(ProxyMetadataPolicyTests, TestStaticMetaStorage_Conversion) {
  pro::detail::static_meta_storage<detail::LargeMeta> from{
      std::in_place_type<detail::Tag<3>>};
  pro::detail::static_meta_storage<detail::SmallMeta> to{};
  to = from;
  ASSERT_EQ(&*to, static_cast<const detail::SmallMeta*>(&*from));
}

TEST(ProxyMetadataPolicyTests, TestInlineMetaStorage_Null) {
  pro::detail::inline_meta_storage<detail::LargeMeta> s{};
  ASSERT_FALSE(s);
  auto copy = s;
  ASSERT_FALSE(copy);
}

TEST(ProxyMetadataPolicyTests, TestInlineMetaStorage_HoldsMetadata) {
  pro::detail::inline_meta_storage<detail::LargeMeta> s{
      std::in_place_type<detail::Tag<2>>};
  ASSERT_TRUE(s);
  ASSERT_EQ((*s).value, 2u);
  ASSERT_EQ((*s).extra, 20u);
  ASSERT_EQ(&*s, static_cast<const detail::LargeMeta*>(&s));
  auto copy = s;
  ASSERT_EQ((*copy).value, 2u);
  ASSERT_NE(&*copy, &*s);
}

TEST(ProxyMetadataPolicyTests, TestInlineMetaStorage_Conversion) {
  pro::detail::inline_meta_storage<detail::SmallMeta> to{};
  to = pro::detail::inline_meta_storage<detail::LargeMeta>{
      std::in_place_type<detail::Tag<3>>};
  ASSERT_EQ((*to).value, 3u);
  to = pro::detail::static_meta_storage<detail::LargeMeta>{
      std::in_place_type<detail::Tag<4>>};
  ASSERT_EQ((*to).value, 4u);
}

TEST(ProxyMetadataPolicyTests, TestInlineMetaStorage_CompositeMetadata) {
  using Storage = pro::detail::inline_meta_storage<detail::CompositeMeta>;
  Storage engaged{std::in_place_type<detail::Tag<2>>};
  Storage empty{};
  Storage copy = engaged;
  ASSERT_TRUE(copy);
  ASSERT_EQ(
      static_cast<const detail::Invoker&>(*copy)(detail::TestContext{3}, 4),
      10u);
  ASSERT_EQ(static_cast<const detail::SmallMeta&>(*copy).value, 2u);
  Storage empty_copy = empty;
  ASSERT_FALSE(empty_copy);
  copy = empty;
  ASSERT_FALSE(copy);
  copy = engaged;
  ASSERT_EQ(static_cast<const detail::SmallMeta&>(*copy).value, 2u);
}
