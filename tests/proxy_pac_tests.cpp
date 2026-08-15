// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#include <proxy/proxy.h>

#ifdef PRO4D_HAS_PAC
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <utility>

namespace proxy_pac_tests_detail {

template <pro::facade F>
constexpr bool IsInlineMetaPreferred = pro::detail::specialization_of<
    pro::compact_facade_meta_traits::storage<pro::detail::proxy_meta<F>>,
    pro::detail::inplace_meta_storage>;

template <class T>
auto GetRawBytes(const T& v) noexcept {
  std::array<std::byte, sizeof(T)> result;
  std::memcpy(result.data(), std::addressof(v), sizeof(T));
  return result;
}

template <pro::facade F>
void CorruptMeta(pro::proxy<F>& p) noexcept {
  // meta_ is the second of the two slots of proxy<F>, behind the storage of
  // the contained value.
  using Storage =
      pro::compact_facade_meta_traits::storage<pro::detail::proxy_meta<F>>;
  static_assert(sizeof(pro::proxy<F>) == sizeof(Storage) + F::max_size);
  std::byte* target =
      reinterpret_cast<std::byte*>(std::addressof(p)) + F::max_size;
  std::uintptr_t word;
  std::memcpy(&word, target, sizeof(word));
  word ^= std::uintptr_t{1} << 54u; // Within the PAC bits for any VA size
  std::memcpy(target, &word, sizeof(word));
}

template <pro::facade F>
void BitwiseCopy(pro::proxy<F>& from, pro::proxy<F>& to) {
  assert(from.has_value() && !to.has_value());
  std::memcpy(static_cast<void*>(&to), static_cast<const void*>(&from),
              sizeof(from));
}

// No conventions and a nothrow destructor: a single signed destroy invoker
// lives inline in the proxy.
struct DefaultFacade : pro::facade_builder::build {};
static_assert(IsInlineMetaPreferred<DefaultFacade>);

// Non-trivial copyability adds more metas than fit inline, so the proxy stores
// a signed pointer to out-of-line static meta storage.
struct CopyableFacade : pro::facade_builder                               //
                        ::support_copy<pro::constraint_level::nontrivial> //
                        ::build {};
static_assert(!IsInlineMetaPreferred<CopyableFacade>);

} // namespace proxy_pac_tests_detail

namespace detail = proxy_pac_tests_detail;

TEST(ProxyPacTests, TestBinaryRepresentation_InlineMetaStorage) {
  pro::proxy<detail::DefaultFacade> p1 = std::make_shared<int>(123);
  auto p1_data = detail::GetRawBytes(p1);
  auto p2 = std::move(p1);
  auto p2_data = detail::GetRawBytes(p2);

  // Pointer authentication signs the stored metadata with address diversity, so
  // a genuine relocation re-signs it and the raw bytes of the signed meta
  // change
  EXPECT_NE(p1_data, p2_data);
}

TEST(ProxyPacTests, TestBinaryRepresentation_StaticMetaStorage) {
  pro::proxy<detail::CopyableFacade> p1 = std::make_shared<int>(123);
  auto p1_data = detail::GetRawBytes(p1);
  auto p2 = p1;
  auto p2_data = detail::GetRawBytes(p2);

  // Both inline metadata and static metadata has pointer authentication
  EXPECT_NE(p1_data, p2_data);
}

TEST(ProxyPacTests, TestBitFlipAttack_InlineMetaStorage) {
  EXPECT_DEATH(
      {
        int a = 123;
        pro::proxy<detail::DefaultFacade> p = &a;
        detail::CorruptMeta(p);
      },
      "");
}

TEST(ProxyPacTests, TestBitFlipAttack_StaticMetaStorage) {
  EXPECT_DEATH(
      {
        int a = 123;
        pro::proxy<detail::CopyableFacade> p = &a;
        detail::CorruptMeta(p);
      },
      "");
}

TEST(ProxyPacTests, TestRelocationAttack_InlineMetaStorage) {
  EXPECT_DEATH(
      {
        int a = 123;
        pro::proxy<detail::DefaultFacade> p1 = &a;
        pro::proxy<detail::DefaultFacade> p2;
        detail::BitwiseCopy(p1, p2);
      },
      "");
}

TEST(ProxyPacTests, TestRelocationAttack_StaticMetaStorage) {
  EXPECT_DEATH(
      {
        int a = 123;
        pro::proxy<detail::CopyableFacade> p1 = &a;
        pro::proxy<detail::CopyableFacade> p2;
        detail::BitwiseCopy(p1, p2);
      },
      "");
}
#endif // PRO4D_HAS_PAC
