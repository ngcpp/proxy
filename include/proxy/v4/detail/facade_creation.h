// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_FACADE_CREATION_H_
#define MSFT_PROXY_V4_DETAIL_FACADE_CREATION_H_

#include <limits>

#include "core.h"

namespace pro::inline v4 {

namespace detail {

inline constexpr std::size_t invalid_size =
    std::numeric_limits<std::size_t>::max();
inline constexpr constraint_level invalid_cl = static_cast<constraint_level>(
    std::numeric_limits<std::underlying_type_t<constraint_level>>::min());
consteval std::size_t merge_size(std::size_t a, std::size_t b) {
  return a < b ? a : b;
}
consteval constraint_level merge_constraint(constraint_level a,
                                            constraint_level b) {
  return a < b ? b : a;
}
consteval std::size_t max_align_of(std::size_t value) {
  value &= ~value + 1u;
  return value < alignof(std::max_align_t) ? value : alignof(std::max_align_t);
}

using ptr_prototype = void* [2];

} // namespace detail

template <class Ss, class Cs, class Rs, std::size_t MaxSize,
          std::size_t MaxAlign, constraint_level Copyability,
          constraint_level Relocatability, constraint_level Destructibility>
struct basic_facade_builder {
  template <class D, detail::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_indirect_convention = basic_facade_builder<
      Ss, detail::composite_t<Cs, detail::conv_impl<false, D, Os>...>, Rs,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, detail::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_direct_convention = basic_facade_builder<
      Ss, detail::composite_t<Cs, detail::conv_impl<true, D, Os>...>, Rs,
      MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, detail::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_convention = add_indirect_convention<D, Os...>;
  template <class R>
  using add_indirect_reflection = basic_facade_builder<
      Ss, Cs, detail::composite_t<Rs, detail::refl_impl<false, R>>, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class R>
  using add_direct_reflection = basic_facade_builder<
      Ss, Cs, detail::composite_t<Rs, detail::refl_impl<true, R>>, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class R>
  using add_reflection = add_indirect_reflection<R>;
  template <facade F, bool Unused = false>
  using add_facade = basic_facade_builder<
      detail::composite_t<Ss, F>, Cs, Rs,
      detail::merge_size(MaxSize, F::max_size),
      detail::merge_size(MaxAlign, F::max_align),
      detail::merge_constraint(Copyability, F::copyability),
      detail::merge_constraint(Relocatability, F::relocatability),
      detail::merge_constraint(Destructibility, F::destructibility)>;
  template <facade F>
  using add_facade_with_substitution = add_facade<F>;
  template <std::size_t PtrSize,
            std::size_t PtrAlign = detail::max_align_of(PtrSize)>
    requires(detail::is_layout_well_formed(PtrSize, PtrAlign))
  using restrict_layout =
      basic_facade_builder<Ss, Cs, Rs, detail::merge_size(MaxSize, PtrSize),
                           detail::merge_size(MaxAlign, PtrAlign), Copyability,
                           Relocatability, Destructibility>;
  template <constraint_level CL>
    requires(detail::is_cl_well_formed(CL))
  using support_copy =
      basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign,
                           detail::merge_constraint(Copyability, CL),
                           Relocatability, Destructibility>;
  template <constraint_level CL>
    requires(detail::is_cl_well_formed(CL))
  using support_relocation =
      basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability,
                           detail::merge_constraint(Relocatability, CL),
                           Destructibility>;
  template <constraint_level CL>
    requires(detail::is_cl_well_formed(CL))
  using support_destruction =
      basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability,
                           Relocatability,
                           detail::merge_constraint(Destructibility, CL)>;
  template <template <class> class Skill>
  using add_skill = Skill<basic_facade_builder>;
  using build = detail::facade_impl<
      Ss, Cs, Rs,
      MaxSize == detail::invalid_size ? sizeof(detail::ptr_prototype) : MaxSize,
      MaxAlign == detail::invalid_size ? alignof(detail::ptr_prototype)
                                       : MaxAlign,
      Copyability == detail::invalid_cl ? constraint_level::none : Copyability,
      Relocatability == detail::invalid_cl ? constraint_level::trivial
                                           : Relocatability,
      Destructibility == detail::invalid_cl ? constraint_level::nothrow
                                            : Destructibility>;
  basic_facade_builder() = delete;
};
using facade_builder =
    basic_facade_builder<std::tuple<>, std::tuple<>, std::tuple<>,
                         detail::invalid_size, detail::invalid_size,
                         detail::invalid_cl, detail::invalid_cl,
                         detail::invalid_cl>;

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_FACADE_CREATION_H_
