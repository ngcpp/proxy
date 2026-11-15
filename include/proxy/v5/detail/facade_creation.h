// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V5_DETAIL_FACADE_CREATION_H_
#define MSFT_PROXY_V5_DETAIL_FACADE_CREATION_H_

#include <limits>

#include "core.h"

namespace pro::inline v5 {

namespace detail {

inline constexpr std::size_t invalid_size =
    std::numeric_limits<std::size_t>::max();
inline constexpr lifetime_requirement invalid_lr = static_cast<
    lifetime_requirement>(
    std::numeric_limits<std::underlying_type_t<lifetime_requirement>>::min());
consteval std::size_t merge_size(std::size_t a, std::size_t b) {
  return a < b ? a : b;
}
consteval lifetime_requirement merge_lr(lifetime_requirement a,
                                        lifetime_requirement b) {
  return a < b ? b : a;
}
consteval std::size_t max_align_of(std::size_t value) {
  value &= ~value + 1u;
  return value < alignof(std::max_align_t) ? value : alignof(std::max_align_t);
}

using ptr_prototype = void* [2];

template <bool IsDirect, class D, class S>
struct legacy_op_impl : op_impl<IsDirect, D, S, default_access_t<D>> {};
template <bool IsDirect, class M>
struct legacy_refl_impl : refl_impl<IsDirect, M, default_access_t<M>> {};

} // namespace detail

template <class Fs, class Os, class Rs, std::size_t MaxSize,
          std::size_t MaxAlign, lifetime_requirement Copyability,
          lifetime_requirement Relocatability,
          lifetime_requirement Destructibility>
struct basic_facade_builder {
  template <class D, detail::extended_signature... Ss>
    requires(sizeof...(Ss) > 0u)
  using add_indirect_convention = basic_facade_builder<
      Fs, detail::composite_t<Os, detail::legacy_op_impl<false, D, Ss>...>, Rs,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, detail::extended_signature... Ss>
    requires(sizeof...(Ss) > 0u)
  using add_direct_convention = basic_facade_builder<
      Fs, detail::composite_t<Os, detail::legacy_op_impl<true, D, Ss>...>, Rs,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, detail::extended_signature... Ss>
    requires(sizeof...(Ss) > 0u)
  using add_convention = add_indirect_convention<D, Ss...>;
  template <class M>
  using add_indirect_reflection = basic_facade_builder<
      Fs, Os, detail::composite_t<Rs, detail::legacy_refl_impl<false, M>>,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class M>
  using add_direct_reflection = basic_facade_builder<
      Fs, Os, detail::composite_t<Rs, detail::legacy_refl_impl<true, M>>,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class M>
  using add_reflection = add_indirect_reflection<M>;
  template <facade F, bool Unused = false>
  using add_facade =
      basic_facade_builder<detail::composite_t<Fs, F>, Os, Rs,
                           detail::merge_size(MaxSize, F::max_size),
                           detail::merge_size(MaxAlign, F::max_align),
                           detail::merge_lr(Copyability, F::copyability),
                           detail::merge_lr(Relocatability, F::relocatability),
                           detail::merge_lr(Destructibility,
                                            F::destructibility)>;
  template <facade F>
  using add_facade_with_substitution = add_facade<F>;
  template <std::size_t PtrSize,
            std::size_t PtrAlign = detail::max_align_of(PtrSize)>
    requires(detail::is_layout_well_formed(PtrSize, PtrAlign))
  using restrict_layout =
      basic_facade_builder<Fs, Os, Rs, detail::merge_size(MaxSize, PtrSize),
                           detail::merge_size(MaxAlign, PtrAlign), Copyability,
                           Relocatability, Destructibility>;
  template <lifetime_requirement LR>
    requires(detail::is_lr_well_formed(LR))
  using support_copy = basic_facade_builder<Fs, Os, Rs, MaxSize, MaxAlign,
                                            detail::merge_lr(Copyability, LR),
                                            Relocatability, Destructibility>;
  template <lifetime_requirement LR>
    requires(detail::is_lr_well_formed(LR))
  using support_relocation =
      basic_facade_builder<Fs, Os, Rs, MaxSize, MaxAlign, Copyability,
                           detail::merge_lr(Relocatability, LR),
                           Destructibility>;
  template <lifetime_requirement LR>
    requires(detail::is_lr_well_formed(LR))
  using support_destruction =
      basic_facade_builder<Fs, Os, Rs, MaxSize, MaxAlign, Copyability,
                           Relocatability,
                           detail::merge_lr(Destructibility, LR)>;
  template <template <class> class Skill>
  using add_skill = Skill<basic_facade_builder>;
  using build = detail::facade_impl<
      Fs, Os, Rs,
      MaxSize == detail::invalid_size ? sizeof(detail::ptr_prototype) : MaxSize,
      MaxAlign == detail::invalid_size ? alignof(detail::ptr_prototype)
                                       : MaxAlign,
      Copyability == detail::invalid_lr ? lifetime_requirement::none
                                        : Copyability,
      Relocatability == detail::invalid_lr ? lifetime_requirement::trivial
                                           : Relocatability,
      Destructibility == detail::invalid_lr ? lifetime_requirement::nothrow
                                            : Destructibility>;
  basic_facade_builder() = delete;
};
using facade_builder =
    basic_facade_builder<std::tuple<>, std::tuple<>, std::tuple<>,
                         detail::invalid_size, detail::invalid_size,
                         detail::invalid_lr, detail::invalid_lr,
                         detail::invalid_lr>;

} // namespace pro::inline v5

#endif // MSFT_PROXY_V5_DETAIL_FACADE_CREATION_H_
