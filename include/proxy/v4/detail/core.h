// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_DETAIL_CORE_H_
#define MSFT_PROXY_V4_DETAIL_CORE_H_

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../proxy_macros.h"

#if __has_cpp_attribute(msvc::no_unique_address)
#define PRO4D_NO_UNIQUE_ADDRESS_ATTRIBUTE msvc::no_unique_address
#elif __has_cpp_attribute(no_unique_address)
#define PRO4D_NO_UNIQUE_ADDRESS_ATTRIBUTE no_unique_address
#else
#error Proxy requires C++20 attribute no_unique_address.
#endif // __has_cpp_attribute(msvc::no_unique_address)

#if __cpp_lib_unreachable >= 202202L
#define PRO4D_UNREACHABLE() std::unreachable()
#else
#define PRO4D_UNREACHABLE() std::abort()
#endif // __cpp_lib_unreachable >= 202202L

namespace pro::inline v4 {

namespace detail {

template <class F>
struct basic_facade_traits;

} // namespace detail

enum class constraint_level { none, nontrivial, nothrow, trivial };

template <template <class> class O>
struct facade_aware_overload_t {
  facade_aware_overload_t() = delete;
};

template <class F>
concept facade = detail::basic_facade_traits<F>::applicable;

template <facade F>
class proxy_indirect_accessor;
template <facade F>
class PRO4D_ENFORCE_EBO proxy;

template <class T>
struct is_bitwise_trivially_relocatable
    : std::bool_constant<std::is_trivially_move_constructible_v<T> &&
                         std::is_trivially_destructible_v<T>> {};
template <class T>
constexpr bool is_bitwise_trivially_relocatable_v =
    is_bitwise_trivially_relocatable<T>::value;

namespace detail {

struct applicable_traits {
  static constexpr bool applicable = true;
};
struct inapplicable_traits {
  static constexpr bool applicable = false;
};

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    : recursive_reduction<R, R<O, I>, Is...> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;

template <template <class...> class R, class... Args>
struct reduction_traits {
  template <class O, class I>
  using type = typename R<Args..., O, I>::type;
};

template <class O, class I>
struct composition_reduction : std::type_identity<O> {};
template <template <class...> class T, class... Os, class I>
  requires(!std::is_void_v<I>)
struct composition_reduction<T<Os...>, I> : std::type_identity<T<Os..., I>> {};
template <template <class...> class T, class... Os, class... Is>
struct composition_reduction<T<Os...>, T<Is...>>
    : std::type_identity<T<Os..., Is...>> {};
template <class T, class... Us>
using composite_t = recursive_reduction_t<
    reduction_traits<composition_reduction>::template type, T, Us...>;

template <class Expr>
consteval bool is_consteval(Expr) {
  return requires { typename std::bool_constant<(Expr{}(), false)>; };
}
template <class T, class U>
concept static_prop = std::is_same_v<T, const U&>;

template <class T, std::size_t I>
concept has_tuple_element = requires { typename std::tuple_element_t<I, T>; };
template <class T>
consteval bool is_tuple_like_well_formed() {
  if constexpr (requires {
                  { std::tuple_size<T>::value } -> static_prop<std::size_t>;
                }) {
    if constexpr (is_consteval([] { return std::tuple_size<T>::value; })) {
      return []<std::size_t... I>(std::index_sequence<I...>) {
        return (has_tuple_element<T, I> && ...);
      }(std::make_index_sequence<std::tuple_size_v<T>>{});
    }
  }
  return false;
}

template <template <class...> class T, class TL, class Is, class... Args>
struct specialization_type_traits_impl;
template <template <class...> class T, class TL, std::size_t... Is,
          class... Args>
struct specialization_type_traits_impl<T, TL, std::index_sequence<Is...>,
                                       Args...>
    : std::type_identity<T<Args..., std::tuple_element_t<Is, TL>...>> {};
template <template <class...> class T, class TL, class... Args>
struct specialization_type_traits
    : specialization_type_traits_impl<
          T, TL, std::make_index_sequence<std::tuple_size_v<TL>>, Args...> {};
template <template <class...> class T, class... Ts, class... Args>
struct specialization_type_traits<T, std::tuple<Ts...>, Args...>
    : std::type_identity<T<Args..., Ts...>> {};
template <template <class...> class T, class TL, class... Args>
using specialization_t =
    typename specialization_type_traits<T, TL, Args...>::type;

enum class qualifier_type { lv, const_lv, rv, const_rv };
template <class T, qualifier_type Q>
struct add_qualifier_traits;
template <class T>
struct add_qualifier_traits<T, qualifier_type::lv> : std::type_identity<T&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::const_lv>
    : std::type_identity<const T&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::rv> : std::type_identity<T&&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::const_rv>
    : std::type_identity<const T&&> {};
template <class T, qualifier_type Q>
using add_qualifier_t = typename add_qualifier_traits<T, Q>::type;
template <class T, qualifier_type Q>
using add_qualifier_ptr_t = std::remove_reference_t<add_qualifier_t<T, Q>>*;

template <class T, constraint_level CL>
struct copyability_traits : inapplicable_traits {};
template <class T>
struct copyability_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires(std::is_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::nontrivial> : applicable_traits {
};
template <class T>
  requires(std::is_nothrow_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::nothrow> : applicable_traits {};
template <class T>
  requires(std::is_trivially_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::trivial> : applicable_traits {};

template <class T, constraint_level CL>
struct relocatability_traits : inapplicable_traits {};
template <class T>
struct relocatability_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires((std::is_move_constructible_v<T> && std::is_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::nontrivial>
    : applicable_traits {};
template <class T>
  requires((std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::nothrow> : applicable_traits {
};
template <class T>
  requires(is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::trivial> : applicable_traits {
};

template <class T, constraint_level CL>
struct destructibility_traits : inapplicable_traits {};
template <class T>
struct destructibility_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires(std::is_destructible_v<T>)
struct destructibility_traits<T, constraint_level::nontrivial>
    : applicable_traits {};
template <class T>
  requires(std::is_nothrow_destructible_v<T>)
struct destructibility_traits<T, constraint_level::nothrow>
    : applicable_traits {};
template <class T>
  requires(std::is_trivially_destructible_v<T>)
struct destructibility_traits<T, constraint_level::trivial>
    : applicable_traits {};

template <class F, bool IsDirect, qualifier_type Q>
using proxy_accessor = add_qualifier_t<
    std::conditional_t<IsDirect, proxy<F>, proxy_indirect_accessor<F>>, Q>;
template <class F, qualifier_type Q>
add_qualifier_t<proxy<F>, Q>
    as_proxy(add_qualifier_t<proxy_indirect_accessor<F>, Q> p);

struct proxy_helper {
  template <class P, class F>
  struct resetting_guard {
    explicit resetting_guard(proxy<F>& p) noexcept : p_(p) {}
    explicit resetting_guard(proxy_indirect_accessor<F>& p) noexcept
        : p_(as_proxy<F, qualifier_type::lv>(p)) {}
    ~resetting_guard() noexcept(std::is_nothrow_destructible_v<P>) {
      std::destroy_at(std::addressof(get_ptr<P, F, qualifier_type::lv>(p_)));
      p_.meta_.reset();
    }

  private:
    proxy<F>& p_;
  };

  template <class M, class F>
  static const M& get_meta(const proxy<F>& p) noexcept {
    assert(p.has_value());
    return static_cast<const M&>(*p.meta_.operator->());
  }
  template <class M, class F>
  static const M& get_meta(const proxy_indirect_accessor<F>& p) noexcept {
    return get_meta<M>(as_proxy<F, qualifier_type::const_lv>(p));
  }
  template <class P, class F, qualifier_type Q>
  static add_qualifier_t<P, Q> get_ptr(add_qualifier_t<proxy<F>, Q> p) {
    return static_cast<add_qualifier_t<P, Q>>(
        *std::launder(reinterpret_cast<add_qualifier_ptr_t<P, Q>>(p.ptr_)));
  }
  template <class P, class F1, class F2>
  static void trivially_relocate(proxy<F1>& from, proxy<F2>& to) noexcept {
    std::uninitialized_copy_n(from.ptr_, sizeof(P), to.ptr_);
    to.meta_ = decltype(proxy<F2>::meta_){std::in_place_type<P>};
    from.meta_.reset();
  }
};

template <class P, bool IsDirect, qualifier_type Q>
struct operand_traits : add_qualifier_traits<P, Q> {};
template <class P, qualifier_type Q>
struct operand_traits<P, false, Q>
    : std::type_identity<decltype(*std::declval<add_qualifier_t<P, Q>>())> {};
template <class P, bool IsDirect, qualifier_type Q>
using operand_t = typename operand_traits<P, IsDirect, Q>::type;
template <class P, bool IsDirect, class D, qualifier_type Q, bool NE, class R,
          class... Args>
concept invocable_dispatch =
    (IsDirect || (requires { *std::declval<add_qualifier_t<P, Q>>(); } &&
                  (!NE || noexcept(*std::declval<add_qualifier_t<P, Q>>())))) &&
    ((NE && std::is_nothrow_invocable_r_v<R, D, operand_t<P, IsDirect, Q>,
                                          Args...>) ||
     (!NE &&
      std::is_invocable_r_v<R, D, operand_t<P, IsDirect, Q>, Args...>)) &&
    (Q != qualifier_type::rv || (NE && std::is_nothrow_destructible_v<P>) ||
     (!NE && std::is_destructible_v<P>));

struct internal_dispatch {};

template <class O>
struct overload_traits : inapplicable_traits {};
template <qualifier_type Q, bool NE, class R, class... Args>
struct overload_traits_impl : applicable_traits {
  using return_type = R;

  template <class P, bool IsDirect, class D>
  static constexpr bool applicable_ptr =
      invocable_dispatch<P, IsDirect, D, Q, NE, R, Args...>;
};
template <class R, class... Args>
struct overload_traits<R(Args...)>
    : overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) noexcept>
    : overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) &>
    : overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) & noexcept>
    : overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) &&>
    : overload_traits_impl<qualifier_type::rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) && noexcept>
    : overload_traits_impl<qualifier_type::rv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const>
    : overload_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const noexcept>
    : overload_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&>
    : overload_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const & noexcept>
    : overload_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&&>
    : overload_traits_impl<qualifier_type::const_rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const && noexcept>
    : overload_traits_impl<qualifier_type::const_rv, true, R, Args...> {};
template <class O>
using ret_t = typename overload_traits<O>::return_type;

template <class O>
struct overload_substitution_traits : inapplicable_traits {
  template <class>
  using type = O;
};
template <template <class> class O>
struct overload_substitution_traits<facade_aware_overload_t<O>>
    : applicable_traits {
  template <class F>
  using type = O<F>;
};
template <class O, class F>
using substituted_overload_t =
    typename overload_substitution_traits<O>::template type<F>;
template <class O>
concept extended_overload = overload_traits<O>::applicable ||
                            overload_substitution_traits<O>::applicable;
template <class P, class F, bool IsDirect, class D, class O>
consteval void diagnose_proxiable_required_convention_not_implemented() {
  static_assert(overload_traits<O>::applicable &&
                    overload_traits<O>::template applicable_ptr<P, IsDirect, D>,
                "not proxiable due to a required convention not implemented");
}

template <class ProP, class D, class O>
struct conv_meta;
#define PRO4D_DEF_CONV_META(oq, pq, ne, ...)                                   \
  template <class ProP, class D, class R, class... Args>                       \
  struct conv_meta<ProP, D, R(Args...) oq ne> {                                \
    conv_meta() = default;                                                     \
    template <class P>                                                         \
    constexpr explicit conv_meta(std::in_place_type_t<P>)                      \
        : invoke([](ProP pq self, Args... args) ne -> R {                      \
            return reinterpret_invoke<P, D, R>(static_cast<ProP pq>(self),     \
                                               std::forward<Args>(args)...);   \
          }) {}                                                                \
                                                                               \
    R (*invoke)(ProP pq, Args...) ne;                                          \
  }
PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(PRO4D_DEF_CONV_META)
#undef PRO4D_DEF_CONV_META

template <class... Ms>
struct PRO4D_ENFORCE_EBO composite_meta : Ms... {
  composite_meta() = default;
  template <class P>
  constexpr explicit composite_meta(std::in_place_type_t<P>)
      : Ms(std::in_place_type<P>)... {}
};

template <class T>
consteval bool is_is_direct_well_formed() {
  if constexpr (requires {
                  { T::is_direct } -> static_prop<bool>;
                }) {
    if constexpr (is_consteval([] { return T::is_direct; })) {
      return true;
    }
  }
  return false;
}

template <class C, class... Os>
struct basic_conv_traits_impl : inapplicable_traits {};
template <class C, extended_overload... Os>
  requires(sizeof...(Os) > 0u)
struct basic_conv_traits_impl<C, Os...> : applicable_traits {};
template <class C>
struct basic_conv_traits : inapplicable_traits {};
template <class C>
  requires(requires {
    { typename C::dispatch_type() } noexcept;
    typename C::overload_types;
  } && is_is_direct_well_formed<C>() &&
           is_tuple_like_well_formed<typename C::overload_types>())
struct basic_conv_traits<C>
    : specialization_t<basic_conv_traits_impl, typename C::overload_types, C> {
};

template <class T>
struct a11y_traits_impl
    : std::conditional<std::is_nothrow_default_constructible_v<T> &&
                           std::is_trivially_copyable_v<T> &&
                           !std::is_final_v<T>,
                       T, void> {};
template <class SFINAE, class T, class... Args>
struct a11y_traits : std::type_identity<void> {};
template <class T, class... Args>
struct a11y_traits<std::void_t<typename T::template accessor<Args...>>, T,
                   Args...>
    : a11y_traits_impl<typename T::template accessor<Args...>> {};
template <class T, class... Args>
using accessor_t = typename a11y_traits<void, T, Args...>::type;

template <class C, class F, class... Os>
struct conv_traits_impl {
  static_assert((overload_traits<substituted_overload_t<Os, F>>::applicable &&
                 ...));
  using meta = composite_meta<conv_meta<
      std::conditional_t<C::is_direct, proxy<F>, proxy_indirect_accessor<F>>,
      typename C::dispatch_type, substituted_overload_t<Os, F>>...>;
  template <class T>
  using accessor =
      accessor_t<typename C::dispatch_type, T, typename C::dispatch_type,
                 substituted_overload_t<Os, F>...>;

  template <class P>
  static consteval void diagnose_proxiable() {
    ((diagnose_proxiable_required_convention_not_implemented<
         P, F, C::is_direct, typename C::dispatch_type,
         substituted_overload_t<Os, F>>()),
     ...);
  }

  template <class P>
  static constexpr bool applicable_ptr =
      (overload_traits<substituted_overload_t<Os, F>>::template applicable_ptr<
           P, C::is_direct, typename C::dispatch_type> &&
       ...);
};
template <class C, class F>
struct conv_traits
    : specialization_t<conv_traits_impl, typename C::overload_types, C, F> {};

template <bool IsDirect, class R>
struct reflection_meta {
  reflection_meta() = default;
  template <class P>
    requires(IsDirect)
  constexpr explicit reflection_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<P>) {}
  template <class P>
    requires(!IsDirect)
  constexpr explicit reflection_meta(std::in_place_type_t<P>)
      : reflector(
            std::in_place_type<typename std::pointer_traits<P>::element_type>) {
  }

  [[PRO4D_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  R reflector;
};

template <class T, bool IsDirect, class R>
consteval bool is_reflector_well_formed() {
  if constexpr (IsDirect) {
    if constexpr (std::is_constructible_v<R, std::in_place_type_t<T>>) {
      if constexpr (is_consteval([] { return R(std::in_place_type<T>); })) {
        return true;
      }
    }
  } else {
    return is_reflector_well_formed<
        typename std::pointer_traits<T>::element_type, true, R>();
  }
  return false;
}
template <class P, class F, bool IsDirect, class R>
consteval void diagnose_proxiable_required_reflection_not_implemented() {
  static_assert(is_reflector_well_formed<P, IsDirect, R>(),
                "not proxiable due to a required reflection not implemented");
}

struct copy_dispatch {
  template <class T, class F>
  PRO4D_STATIC_CALL(void, const T& self, proxy<F>& rhs) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    std::construct_at(std::addressof(rhs), self);
  }
};
struct relocate_dispatch : internal_dispatch {
  template <class P, class F>
  PRO4D_STATIC_CALL(void, std::in_place_type_t<P>, proxy<F>&& self,
                    proxy<F>& rhs) noexcept {
    proxy_helper::trivially_relocate<P>(self, rhs);
  }
  template <class T, class F>
  PRO4D_STATIC_CALL(void, T&& self, proxy<F>& rhs) noexcept(
      relocatability_traits<T, constraint_level::nothrow>::applicable) {
    std::construct_at(std::addressof(rhs), std::forward<T>(self));
  }
};
struct destroy_dispatch {
  template <class T>
  PRO4D_STATIC_CALL(void, T& self) noexcept(std::is_nothrow_destructible_v<T>) {
    std::destroy_at(&self);
  }
};
template <class F, class D, class ONE, class OE, constraint_level C>
struct lifetime_meta_traits : std::type_identity<void> {};
template <class F, class D, class ONE, class OE>
struct lifetime_meta_traits<F, D, ONE, OE, constraint_level::nothrow>
    : std::type_identity<conv_meta<proxy<F>, D, ONE>> {};
template <class F, class D, class ONE, class OE>
struct lifetime_meta_traits<F, D, ONE, OE, constraint_level::nontrivial>
    : std::type_identity<conv_meta<proxy<F>, D, OE>> {};
template <class F, class D, class ONE, class OE, constraint_level C>
using lifetime_meta_t = typename lifetime_meta_traits<F, D, ONE, OE, C>::type;

template <class... As>
struct PRO4D_ENFORCE_EBO composite_accessor : As... {};

template <class C, class F, bool IsDirect>
struct conv_accessor_traits : std::type_identity<void> {};
template <class C, class F>
  requires(!C::is_direct)
struct conv_accessor_traits<C, F, false>
    : std::type_identity<typename conv_traits<C, F>::template accessor<
          proxy_indirect_accessor<F>>> {};
template <class C, class F>
  requires(C::is_direct)
struct conv_accessor_traits<C, F, true>
    : std::type_identity<
          typename conv_traits<C, F>::template accessor<proxy<F>>> {};
template <class C, class F, bool IsDirect>
using conv_accessor_t = typename conv_accessor_traits<C, F, IsDirect>::type;

template <class R, class F, bool IsDirect>
struct refl_accessor_traits : std::type_identity<void> {};
template <class R, class F>
  requires(!R::is_direct)
struct refl_accessor_traits<R, F, false>
    : std::type_identity<
          accessor_t<typename R::reflector_type, proxy_indirect_accessor<F>,
                     typename R::reflector_type>> {};
template <class R, class F>
  requires(R::is_direct)
struct refl_accessor_traits<R, F, true>
    : std::type_identity<accessor_t<typename R::reflector_type, proxy<F>,
                                    typename R::reflector_type>> {};
template <class R, class F, bool IsDirect>
using refl_accessor_t = typename refl_accessor_traits<R, F, IsDirect>::type;

template <class T>
concept pointer_like = (std::is_pointer_v<T> ||
    requires { typename T::element_type; } || requires(T val) { *val; }) &&
    requires { typename std::pointer_traits<T>::element_type; };

template <class T, template <class...> class TT>
struct specialization_traits : inapplicable_traits {};
template <template <class...> class TT, class... Args>
struct specialization_traits<TT<Args...>, TT> : applicable_traits {};
template <class T, template <class...> class TT>
concept specialization_of = specialization_traits<T, TT>::applicable;

template <class P, class F, std::size_t ActualSize, std::size_t MaxSize>
consteval void diagnose_proxiable_size_too_large() {
  static_assert(ActualSize <= MaxSize, "not proxiable due to size too large");
}
template <class P, class F, std::size_t ActualAlign, std::size_t MaxAlign>
consteval void diagnose_proxiable_align_too_large() {
  static_assert(ActualAlign <= MaxAlign,
                "not proxiable due to alignment too large");
}
template <class P, class F, constraint_level RequiredCopyability>
consteval void diagnose_proxiable_insufficient_copyability() {
  static_assert(copyability_traits<P, RequiredCopyability>::applicable,
                "not proxiable due to insufficient copyability");
}
template <class P, class F, constraint_level RequiredRelocatability>
consteval void diagnose_proxiable_insufficient_relocatability() {
  static_assert(relocatability_traits<P, RequiredRelocatability>::applicable,
                "not proxiable due to insufficient relocatability");
}
template <class P, class F, constraint_level RequiredDestructibility>
consteval void diagnose_proxiable_insufficient_destructibility() {
  static_assert(destructibility_traits<P, RequiredDestructibility>::applicable,
                "not proxiable due to insufficient destructibility");
}

consteval bool is_layout_well_formed(std::size_t size, std::size_t align) {
  return size > 0u && std::has_single_bit(align) && size % align == 0u;
}
consteval bool is_cl_well_formed(constraint_level cl) {
  return cl >= constraint_level::none && cl <= constraint_level::trivial;
}
template <class F>
consteval bool is_facade_constraints_well_formed() {
  if constexpr (requires {
                  { F::max_size } -> static_prop<std::size_t>;
                  { F::max_align } -> static_prop<std::size_t>;
                  { F::copyability } -> static_prop<constraint_level>;
                  { F::relocatability } -> static_prop<constraint_level>;
                  { F::destructibility } -> static_prop<constraint_level>;
                }) {
    if constexpr (is_consteval([] {
                    return std::tuple{F::max_size, F::max_align, F::copyability,
                                      F::relocatability, F::destructibility};
                  })) {
      return is_layout_well_formed(F::max_size, F::max_align) &&
             is_cl_well_formed(F::copyability) &&
             is_cl_well_formed(F::relocatability) &&
             is_cl_well_formed(F::destructibility);
    }
  }
  return false;
}
template <class... Cs>
struct basic_facade_conv_traits_impl : inapplicable_traits {};
template <class... Cs>
  requires(basic_conv_traits<Cs>::applicable && ...)
struct basic_facade_conv_traits_impl<Cs...> : applicable_traits {};
template <class... Rs>
struct basic_facade_refl_traits_impl : inapplicable_traits {};
template <class... Rs>
  requires((requires {
             typename Rs::reflector_type;
           } && is_is_direct_well_formed<Rs>()) && ...)
struct basic_facade_refl_traits_impl<Rs...> : applicable_traits {};
template <class F>
struct basic_facade_traits : inapplicable_traits {};
template <class F>
  requires(requires {
    typename F::convention_types;
    typename F::reflection_types;
  } && is_facade_constraints_well_formed<F>() &&
           is_tuple_like_well_formed<typename F::convention_types>() &&
           specialization_t<basic_facade_conv_traits_impl,
                            typename F::convention_types>::applicable &&
           is_tuple_like_well_formed<typename F::reflection_types>() &&
           specialization_t<basic_facade_refl_traits_impl,
                            typename F::reflection_types>::applicable)
struct basic_facade_traits<F> : applicable_traits {};

template <class F, class... Cs>
struct facade_conv_traits_impl {
  using conv_meta =
      composite_t<composite_meta<>, typename conv_traits<Cs, F>::meta...>;
  using conv_indirect_accessor =
      composite_t<composite_accessor<>, conv_accessor_t<Cs, F, false>...>;
  using conv_direct_accessor =
      composite_t<composite_accessor<>, conv_accessor_t<Cs, F, true>...>;

  template <class P>
  static consteval void diagnose_proxiable_conv() {
    (conv_traits<Cs, F>::template diagnose_proxiable<P>(), ...);
  }

  template <class P>
  static constexpr bool conv_applicable_ptr =
      (conv_traits<Cs, F>::template applicable_ptr<P> && ...);
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using refl_meta = composite_meta<
      reflection_meta<Rs::is_direct, typename Rs::reflector_type>...>;
  using refl_indirect_accessor =
      composite_t<composite_accessor<>, refl_accessor_t<Rs, F, false>...>;
  using refl_direct_accessor =
      composite_t<composite_accessor<>, refl_accessor_t<Rs, F, true>...>;

  template <class P>
  static consteval void diagnose_proxiable_refl() {
    (diagnose_proxiable_required_reflection_not_implemented<
         P, F, Rs::is_direct, typename Rs::reflector_type>(),
     ...);
  }

  template <class P>
  static constexpr bool refl_applicable_ptr =
      (is_reflector_well_formed<P, Rs::is_direct,
                                typename Rs::reflector_type>() &&
       ...);
};
template <class F>
struct facade_traits : specialization_t<facade_conv_traits_impl,
                                        typename F::convention_types, F>,
                       specialization_t<facade_refl_traits_impl,
                                        typename F::reflection_types, F> {
  using meta = composite_t<
      composite_meta<>,
      lifetime_meta_t<F, copy_dispatch, void(proxy<F>&) const noexcept,
                      void(proxy<F>&) const, F::copyability>,
      lifetime_meta_t<F, relocate_dispatch, void(proxy<F>&) && noexcept,
                      void(proxy<F>&) &&, F::relocatability>,
      lifetime_meta_t<F, destroy_dispatch, void() noexcept, void(),
                      F::destructibility>,
      typename facade_traits::conv_meta, typename facade_traits::refl_meta>;
  using indirect_accessor =
      composite_t<typename facade_traits::conv_indirect_accessor,
                  typename facade_traits::refl_indirect_accessor>;
  using direct_accessor =
      composite_t<typename facade_traits::conv_direct_accessor,
                  typename facade_traits::refl_direct_accessor>;

  template <class P>
  [[noreturn]] static consteval void diagnose_proxiable_noreturn() {
    diagnose_proxiable_size_too_large<P, F, sizeof(P), F::max_size>();
    diagnose_proxiable_align_too_large<P, F, alignof(P), F::max_align>();
    diagnose_proxiable_insufficient_copyability<P, F, F::copyability>();
    diagnose_proxiable_insufficient_relocatability<P, F, F::relocatability>();
    diagnose_proxiable_insufficient_destructibility<P, F, F::destructibility>();
    facade_traits::template diagnose_proxiable_conv<P>();
    facade_traits::template diagnose_proxiable_refl<P>();
    PRO4D_UNREACHABLE(); // Propagate the error to the caller side
  }

  template <class P>
  static constexpr bool applicable_ptr =
      sizeof(P) <= F::max_size && alignof(P) <= F::max_align &&
      copyability_traits<P, F::copyability>::applicable &&
      relocatability_traits<P, F::relocatability>::applicable &&
      destructibility_traits<P, F::destructibility>::applicable &&
      facade_traits::template conv_applicable_ptr<P> &&
      facade_traits::template refl_applicable_ptr<P>;
};

using ptr_prototype = void* [2];

template <class M>
struct meta_ptr_indirect_impl {
  meta_ptr_indirect_impl() = default;
  template <class P>
  explicit meta_ptr_indirect_impl(std::in_place_type_t<P>)
      : ptr_(&storage<P>) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  const M* operator->() const noexcept { return ptr_; }

private:
  const M* ptr_;
  template <class P>
  static constexpr M storage{std::in_place_type<P>};
};
template <class M, class DM>
struct meta_ptr_direct_impl : private M {
  using M::M;
  bool has_value() const noexcept { return this->DM::invoke != nullptr; }
  void reset() noexcept { this->DM::invoke = nullptr; }
  const M* operator->() const noexcept { return this; }
};
template <class M>
struct meta_ptr_traits_impl : std::type_identity<meta_ptr_indirect_impl<M>> {};
template <class ProP, class D, class O, class... Ms>
struct meta_ptr_traits_impl<composite_meta<conv_meta<ProP, D, O>, Ms...>>
    : std::type_identity<
          meta_ptr_direct_impl<composite_meta<conv_meta<ProP, D, O>, Ms...>,
                               conv_meta<ProP, D, O>>> {};
template <class M>
struct meta_ptr_traits : std::type_identity<meta_ptr_indirect_impl<M>> {};
template <class M>
  requires(sizeof(M) <= sizeof(ptr_prototype) &&
           alignof(M) <= alignof(ptr_prototype) &&
           std::is_nothrow_default_constructible_v<M> &&
           std::is_trivially_copyable_v<M>)
struct meta_ptr_traits<M> : meta_ptr_traits_impl<M> {};
template <class M>
using meta_ptr = typename meta_ptr_traits<M>::type;

template <class T>
class inplace_ptr {
public:
  template <class... Args>
  explicit inplace_ptr(std::in_place_t, Args&&... args)
      : value_(std::forward<Args>(args)...) {}
  inplace_ptr() = default;
  inplace_ptr(const inplace_ptr&) = default;
  inplace_ptr(inplace_ptr&&) = default;
  inplace_ptr& operator=(const inplace_ptr&) = default;
  inplace_ptr& operator=(inplace_ptr&&) = default;

  T* operator->() noexcept { return std::addressof(value_); }
  const T* operator->() const noexcept { return std::addressof(value_); }
  T& operator*() & noexcept { return value_; }
  const T& operator*() const& noexcept { return value_; }
  T&& operator*() && noexcept { return std::move(value_); }
  const T&& operator*() const&& noexcept { return std::move(value_); }

private:
  [[PRO4D_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  T value_;
};

template <class D, class O, class P, class... Args>
ret_t<O> invoke_impl(P&& p, Args&&... args) {
  return proxy_helper::get_meta<conv_meta<std::remove_cvref_t<P>, D, O>>(p)
      .invoke(std::forward<P>(p), std::forward<Args>(args)...);
}
template <class F, qualifier_type Q>
add_qualifier_t<proxy<F>, Q>
    as_proxy(add_qualifier_t<proxy_indirect_accessor<F>, Q> p) {
  return static_cast<add_qualifier_t<proxy<F>, Q>>(
      *reinterpret_cast<
          add_qualifier_ptr_t<inplace_ptr<proxy_indirect_accessor<F>>, Q>>(
          std::addressof(p)));
}
template <class P, class F, bool IsDirect, qualifier_type Q>
operand_t<P, IsDirect, Q> get_operand(proxy_accessor<F, IsDirect, Q> p) {
  if constexpr (IsDirect) {
    return proxy_helper::get_ptr<P, F, Q>(
        std::forward<proxy_accessor<F, IsDirect, Q>>(p));
  } else {
    add_qualifier_t<P, Q> ptr = proxy_helper::get_ptr<P, F, Q>(
        as_proxy<F, Q>(std::forward<proxy_accessor<F, IsDirect, Q>>(p)));
    if constexpr (std::is_constructible_v<bool, P&>) {
      assert(ptr);
    }
    return *std::forward<add_qualifier_t<P, Q>>(ptr);
  }
}

// When a dispatch always throws, MSVC may incorrectly warn about unreachable
// code (C4702). Disable the warning for invoke_dispatch().
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4702)
#endif // defined(_MSC_VER) && !defined(__clang__)
template <class D, class R, class... Args>
R invoke_dispatch(Args&&... args) {
  if constexpr (std::is_void_v<R>) {
    D()(std::forward<Args>(args)...);
  } else {
    return D()(std::forward<Args>(args)...);
  }
}
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif // defined(_MSC_VER) && !defined(__clang__)
template <class P, class F, bool IsDirect, qualifier_type Q, class D, class R,
          class... Args>
R reinterpret_invoke(proxy_accessor<F, IsDirect, Q> self, Args&&... args) {
  if constexpr (Q == qualifier_type::rv) {
    if constexpr (std::is_base_of_v<internal_dispatch, D> &&
                  is_bitwise_trivially_relocatable_v<P>) {
      return D()(std::in_place_type<P>, std::move(self),
                 std::forward<Args>(args)...);
    } else {
      proxy_helper::resetting_guard<P, F> guard{self};
      return invoke_dispatch<D, R>(
          get_operand<P, F, IsDirect, Q>(std::move(self)),
          std::forward<Args>(args)...);
    }
  } else {
    return invoke_dispatch<D, R>(
        get_operand<P, F, IsDirect, Q>(
            std::forward<proxy_accessor<F, IsDirect, Q>>(self)),
        std::forward<Args>(args)...);
  }
}

} // namespace detail

template <class P, class F>
concept proxiable = facade<F> && detail::pointer_like<P> &&
                    detail::facade_traits<F>::template applicable_ptr<P>;

template <facade F>
class proxy_indirect_accessor
    : public detail::facade_traits<F>::indirect_accessor {
  friend class detail::inplace_ptr<proxy_indirect_accessor>;
  proxy_indirect_accessor() = default;
  proxy_indirect_accessor(const proxy_indirect_accessor&) = default;
  proxy_indirect_accessor& operator=(const proxy_indirect_accessor&) = default;

public:
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(proxy_indirect_accessor& p, Args&&... args) {
    return detail::invoke_impl<D, O>(p, std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(const proxy_indirect_accessor& p,
                                 Args&&... args) {
    return detail::invoke_impl<D, O>(p, std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(proxy_indirect_accessor&& p, Args&&... args) {
    return detail::invoke_impl<D, O>(std::move(p), std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(const proxy_indirect_accessor&& p,
                                 Args&&... args) {
    return detail::invoke_impl<D, O>(std::move(p), std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(proxy_indirect_accessor& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, false, detail::qualifier_type::lv,
                                      D, R>(p, std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(const proxy_indirect_accessor& p,
                              Args&&... args) {
    return detail::reinterpret_invoke<P, F, false,
                                      detail::qualifier_type::const_lv, D, R>(
        p, std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(proxy_indirect_accessor&& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, false, detail::qualifier_type::rv,
                                      D, R>(std::move(p),
                                            std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(const proxy_indirect_accessor&& p,
                              Args&&... args) {
    return detail::reinterpret_invoke<P, F, false,
                                      detail::qualifier_type::const_rv, D, R>(
        std::move(p), std::forward<Args>(args)...);
  }
  template <class R>
  friend const R& reflect(const proxy_indirect_accessor& p) noexcept {
    return detail::proxy_helper::get_meta<detail::reflection_meta<false, R>>(p)
        .reflector;
  }
};

template <facade F>
class proxy : public detail::facade_traits<F>::direct_accessor,
              public detail::inplace_ptr<proxy_indirect_accessor<F>> {
  friend struct detail::proxy_helper;

public:
  using facade_type = F;

  proxy() noexcept { initialize(); }
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy&) noexcept
    requires(F::copyability == constraint_level::trivial)
  = default;
  proxy(const proxy& rhs) noexcept(F::copyability == constraint_level::nothrow)
    requires(F::copyability == constraint_level::nontrivial ||
             F::copyability == constraint_level::nothrow)
      : detail::inplace_ptr<proxy_indirect_accessor<F>>() /* Make GCC happy */ {
    initialize(rhs);
  }
  proxy(proxy&& rhs) noexcept(F::relocatability >= constraint_level::nothrow)
    requires(F::relocatability >= constraint_level::nontrivial &&
             F::copyability != constraint_level::trivial)
  {
    initialize(std::move(rhs));
  }
  template <class P>
  constexpr proxy(P&& ptr) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<P>, P>)
    requires(!detail::specialization_of<std::decay_t<P>, proxy> &&
             detail::pointer_like<std::decay_t<P>> &&
             std::is_constructible_v<std::decay_t<P>, P>)
  {
    initialize<std::decay_t<P>>(std::forward<P>(ptr));
  }
  template <class P, class... Args>
  constexpr explicit proxy(std::in_place_type_t<P>, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, Args...>)
    requires(detail::pointer_like<P> && std::is_constructible_v<P, Args...>)
  {
    initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  constexpr explicit proxy(
      std::in_place_type_t<P>, std::initializer_list<U> il,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       P, std::initializer_list<U>&, Args...>)
    requires(detail::pointer_like<P> &&
             std::is_constructible_v<P, std::initializer_list<U>&, Args...>)
  {
    initialize<P>(il, std::forward<Args>(args)...);
  }
  proxy& operator=(std::nullptr_t) noexcept(F::destructibility >=
                                            constraint_level::nothrow)
    requires(F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return *this;
  }
  proxy& operator=(const proxy&) noexcept
    requires(F::copyability == constraint_level::trivial)
  = default;
  proxy& operator=(const proxy& rhs) noexcept(F::copyability >=
                                                  constraint_level::nothrow &&
                                              F::destructibility >=
                                                  constraint_level::nothrow)
    requires((F::copyability == constraint_level::nontrivial ||
              F::copyability == constraint_level::nothrow) &&
             F::destructibility >= constraint_level::nontrivial)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      if constexpr (F::copyability == constraint_level::nothrow) {
        destroy();
        initialize(rhs);
      } else {
        *this = proxy{rhs};
      }
    }
    return *this;
  }
  proxy& operator=(proxy&& rhs) noexcept(F::relocatability >=
                                             constraint_level::nothrow &&
                                         F::destructibility >=
                                             constraint_level::nothrow)
    requires(F::relocatability >= constraint_level::nontrivial &&
             F::destructibility >= constraint_level::nontrivial &&
             F::copyability != constraint_level::trivial)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      reset();
      initialize(std::move(rhs));
    }
    return *this;
  }
  template <class P>
  constexpr proxy& operator=(P&& ptr) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<P>, P> &&
      F::destructibility >= constraint_level::nothrow)
    requires(!detail::specialization_of<std::decay_t<P>, proxy> &&
             detail::pointer_like<std::decay_t<P>> &&
             std::is_constructible_v<std::decay_t<P>, P> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    if constexpr (std::is_nothrow_constructible_v<std::decay_t<P>, P>) {
      destroy();
      initialize<std::decay_t<P>>(std::forward<P>(ptr));
    } else {
      *this = proxy{std::forward<P>(ptr)};
    }
    return *this;
  }
  ~proxy()
    requires(F::destructibility == constraint_level::trivial)
  = default;
  ~proxy() noexcept(F::destructibility == constraint_level::nothrow)
    requires(F::destructibility == constraint_level::nontrivial ||
             F::destructibility == constraint_level::nothrow)
  {
    destroy();
  }

  bool has_value() const noexcept { return meta_.has_value(); }
  explicit operator bool() const noexcept { return meta_.has_value(); }
  void reset() noexcept(F::destructibility >= constraint_level::nothrow)
    requires(F::destructibility >= constraint_level::nontrivial)
  {
    destroy();
    initialize();
  }
  void swap(proxy& rhs) noexcept(F::relocatability >=
                                     constraint_level::nothrow ||
                                 F::copyability == constraint_level::trivial)
    requires(F::relocatability >= constraint_level::nontrivial ||
             F::copyability == constraint_level::trivial)
  {
    if constexpr (F::relocatability == constraint_level::trivial ||
                  F::copyability == constraint_level::trivial) {
      std::swap(meta_, rhs.meta_);
#ifdef __INTEL_LLVM_COMPILER
      // Workaround: Intel oneAPI compiler (as of 2025.2.0) may over-optimize
      // the swap below, causing unit tests failure
      std::byte temp[F::max_size];
      std::ranges::uninitialized_copy(ptr_, temp);
      std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
      std::ranges::uninitialized_copy(temp, rhs.ptr_);
#else
      std::swap(ptr_, rhs.ptr_);
#endif // __INTEL_LLVM_COMPILER
    } else {
      if (meta_.has_value()) {
        if (rhs.meta_.has_value()) {
          proxy temp = std::move(*this);
          initialize(std::move(rhs));
          rhs.initialize(std::move(temp));
        } else {
          rhs.initialize(std::move(*this));
        }
      } else if (rhs.meta_.has_value()) {
        initialize(std::move(rhs));
      }
    }
  }
  template <class P, class... Args>
  constexpr P& emplace(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, Args...> &&
      F::destructibility >= constraint_level::nothrow)
    requires(detail::pointer_like<P> && std::is_constructible_v<P, Args...> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  constexpr P& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, std::initializer_list<U>&, Args...> &&
      F::destructibility >= constraint_level::nothrow)
    requires(detail::pointer_like<P> &&
             std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return initialize<P>(il, std::forward<Args>(args)...);
  }

  friend void swap(proxy& lhs, proxy& rhs) noexcept(noexcept(lhs.swap(rhs)))
    requires(requires { lhs.swap(rhs); })
  {
    lhs.swap(rhs);
  }
  friend bool operator==(const proxy& lhs, std::nullptr_t) noexcept {
    return !lhs.has_value();
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(proxy& p, Args&&... args) {
    return detail::invoke_impl<D, O>(p, std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(const proxy& p, Args&&... args) {
    return detail::invoke_impl<D, O>(p, std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(proxy&& p, Args&&... args) {
    return detail::invoke_impl<D, O>(std::move(p), std::forward<Args>(args)...);
  }
  template <class D, class O, class... Args>
  friend detail::ret_t<O> invoke(const proxy&& p, Args&&... args) {
    return detail::invoke_impl<D, O>(std::move(p), std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(proxy& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, true, detail::qualifier_type::lv, D,
                                      R>(p, std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(const proxy& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, true,
                                      detail::qualifier_type::const_lv, D, R>(
        p, std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(proxy&& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, true, detail::qualifier_type::rv, D,
                                      R>(std::move(p),
                                         std::forward<Args>(args)...);
  }
  template <class P, class D, class R, class... Args>
  friend R reinterpret_invoke(const proxy&& p, Args&&... args) {
    return detail::reinterpret_invoke<P, F, true,
                                      detail::qualifier_type::const_rv, D, R>(
        std::move(p), std::forward<Args>(args)...);
  }
  template <class R>
  friend const R& reflect(const proxy& p) noexcept {
    return detail::proxy_helper::get_meta<detail::reflection_meta<true, R>>(p)
        .reflector;
  }

private:
  void initialize() {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    meta_.reset();
  }
  void initialize(const proxy& rhs)
    requires(F::copyability != constraint_level::none)
  {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.meta_.has_value()) {
      if constexpr (F::copyability == constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
        meta_ = rhs.meta_;
      } else {
        invoke<detail::copy_dispatch,
               void(proxy&) const noexcept(
                   F::copyability == constraint_level::nothrow)>(rhs, *this);
      }
    } else {
      meta_.reset();
    }
  }
  void initialize(proxy&& rhs)
    requires(F::relocatability != constraint_level::none)
  {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.meta_.has_value()) {
      if constexpr (F::relocatability == constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
        meta_ = rhs.meta_;
        rhs.meta_.reset();
      } else {
        invoke<detail::relocate_dispatch,
               void(proxy&) &&
                   noexcept(F::relocatability == constraint_level::nothrow)>(
            std::move(rhs), *this);
      }
    } else {
      meta_.reset();
    }
  }
  template <class P, class... Args>
  constexpr P& initialize(Args&&... args) {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    P& result = *std::construct_at(reinterpret_cast<P*>(ptr_),
                                   std::forward<Args>(args)...);
    if constexpr (proxiable<P, F>) {
      meta_ = detail::meta_ptr<typename detail::facade_traits<F>::meta>{
          std::in_place_type<P>};
    } else {
      detail::facade_traits<F>::template diagnose_proxiable_noreturn<P>();
    }
    return result;
  }
  void destroy()
    requires(F::destructibility != constraint_level::none)
  {
    if constexpr (F::destructibility != constraint_level::trivial) {
      if (meta_.has_value()) {
        invoke<detail::destroy_dispatch,
               void() noexcept(F::destructibility ==
                               constraint_level::nothrow)>(*this);
      }
    }
  }
  PRO4D_DEBUG(static inline void pro_symbol_guard(proxy& self,
                                                  const proxy& cself) {
    self.operator->();
    *self;
    *std::move(self);
    cself.operator->();
    *cself;
    *std::move(cself);
  })

  detail::meta_ptr<typename detail::facade_traits<F>::meta> meta_;
  alignas(F::max_align) std::byte ptr_[F::max_size];
};

template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(proxy_indirect_accessor<F>& p, Args&&... args) {
  return invoke<D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(const proxy_indirect_accessor<F>& p, Args&&... args) {
  return invoke<D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(proxy_indirect_accessor<F>&& p, Args&&... args) {
  return invoke<D, O>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(const proxy_indirect_accessor<F>&& p, Args&&... args) {
  return invoke<D, O>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(proxy<F>& p, Args&&... args) {
  return invoke<D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(const proxy<F>& p, Args&&... args) {
  return invoke<D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(proxy<F>&& p, Args&&... args) {
  return invoke<D, O>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<O>
    proxy_invoke(const proxy<F>&& p, Args&&... args) {
  return invoke<D, O>(std::move(p), std::forward<Args>(args)...);
}

template <class R, facade F>
[[deprecated("Use unqualified reflect instead")]] const R&
    proxy_reflect(const proxy_indirect_accessor<F>& p) noexcept {
  return reflect<R>(p);
}
template <class R, facade F>
[[deprecated("Use unqualified reflect instead")]] const R&
    proxy_reflect(const proxy<F>& p) noexcept {
  return reflect<R>(p);
}

struct substitution_dispatch;

template <facade F>
struct observer_facade;
template <facade F>
using proxy_view = proxy<observer_facade<F>>;

template <facade F>
struct weak_facade;
template <facade F>
using weak_proxy = proxy<weak_facade<F>>;

namespace detail {

template <class F>
struct converter {
  explicit converter(F f) noexcept : f_(std::move(f)) {}
  converter(const converter&) = delete;
  template <class T>
  operator T() && noexcept(
      std::is_nothrow_invocable_r_v<T, F, std::in_place_type_t<T>>)
    requires(std::is_invocable_r_v<T, F, std::in_place_type_t<T>> &&
             !std::is_invocable_r_v<T, F, std::in_place_type_t<T&>> &&
             !std::is_invocable_r_v<T, F, std::in_place_type_t<T &&>>)
  {
    return std::move(f_)(std::in_place_type<T>);
  }
  template <class T>
  operator T&() && noexcept(
      std::is_nothrow_invocable_r_v<T&, F, std::in_place_type_t<T&>>)
    requires(std::is_invocable_r_v<T&, F, std::in_place_type_t<T&>>)
  {
    return std::move(f_)(std::in_place_type<T&>);
  }
  template <class T>
  operator T&&() && noexcept(
      std::is_nothrow_invocable_r_v<T&&, F, std::in_place_type_t<T&&>>)
    requires(std::is_invocable_r_v<T &&, F, std::in_place_type_t<T &&>>)
  {
    return std::move(f_)(std::in_place_type<T&&>);
  }

private:
  F f_;
};

#define PRO4D_DEF_CAST_ACCESSOR(oq, pq, ne, ...)                               \
  template <class P, class D, class T>                                         \
  struct accessor<P, D, T() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(operator T)                        \
    explicit(Expl) operator T() oq ne {                                        \
      if constexpr (Nullable) {                                                \
        if (!static_cast<const P&>(*this).has_value()) {                       \
          return nullptr;                                                      \
        }                                                                      \
      }                                                                        \
      return invoke<D, T() oq ne>(static_cast<P pq>(*this));                   \
    }                                                                          \
  }
template <bool Expl, bool Nullable>
struct cast_dispatch_base {
  PRO4D_DEF_ACCESSOR_TEMPLATE(
      MEM, PRO4D_DEF_CAST_ACCESSOR,
      operator typename overload_traits<ProOs>::return_type)
};
#undef PRO4D_DEF_CAST_ACCESSOR

template <bool IsDirect, class D, class... Os>
struct conv_impl {
  static constexpr bool is_direct = IsDirect;
  using dispatch_type = D;
  using overload_types = std::tuple<Os...>;
};
template <bool IsDirect, class R>
struct refl_impl {
  static constexpr bool is_direct = IsDirect;
  using reflector_type = R;
};
template <class Cs, class Rs, std::size_t MaxSize, std::size_t MaxAlign,
          constraint_level Copyability, constraint_level Relocatability,
          constraint_level Destructibility>
struct facade_impl {
  using convention_types = Cs;
  using reflection_types = Rs;
  static constexpr std::size_t max_size = MaxSize;
  static constexpr std::size_t max_align = MaxAlign;
  static constexpr constraint_level copyability = Copyability;
  static constexpr constraint_level relocatability = Relocatability;
  static constexpr constraint_level destructibility = Destructibility;
};

template <class O, class I>
struct add_tuple_reduction : std::type_identity<O> {};
template <class... Os, class I>
  requires(!std::is_same_v<I, Os> && ...)
struct add_tuple_reduction<std::tuple<Os...>, I>
    : std::type_identity<std::tuple<Os..., I>> {};
template <class O, class... Is>
using add_tuple_t =
    recursive_reduction_t<reduction_traits<add_tuple_reduction>::template type,
                          O, Is...>;

template <bool IsDirect, class D>
struct conv_specialization_helper {
  template <class... Os>
  using type = conv_impl<IsDirect, D, Os...>;
};
template <bool IsDirect, class D, class Os>
using conv_specialization_t =
    specialization_t<conv_specialization_helper<IsDirect, D>::template type,
                     Os>;

template <class LR, class CLR, class RR, class CRR>
class observer_ptr {
public:
  explicit observer_ptr(LR lr) : lr_(lr) {}
  observer_ptr(const observer_ptr&) = default;
  auto operator->() noexcept { return std::addressof(lr_); }
  auto operator->() const noexcept {
    return std::addressof(static_cast<CLR>(lr_));
  }
  LR operator*() & noexcept { return static_cast<LR>(lr_); }
  CLR operator*() const& noexcept { return static_cast<CLR>(lr_); }
  RR operator*() && noexcept { return static_cast<RR>(lr_); }
  CRR operator*() const&& noexcept { return static_cast<CRR>(lr_); }

private:
  LR lr_;
};

template <class O>
using observer_substitution_overload =
    proxy_view<typename ret_t<O>::facade_type>() const noexcept;
template <class... Os>
using observer_substitution_conv = conv_specialization_t<
    true, substitution_dispatch,
    add_tuple_t<std::tuple<>, observer_substitution_overload<Os>...>>;

template <class C>
struct observer_conv_traits : std::type_identity<void> {};
template <class C>
  requires(C::is_direct &&
           std::is_same_v<typename C::dispatch_type, substitution_dispatch>)
struct observer_conv_traits<C>
    : std::type_identity<specialization_t<observer_substitution_conv,
                                          typename C::overload_types>> {};
template <class C>
  requires(!C::is_direct)
struct observer_conv_traits<C> : std::type_identity<C> {};
template <class... Cs>
using observer_conv_types =
    composite_t<std::tuple<>, typename observer_conv_traits<Cs>::type...>;
template <class... Rs>
using observer_refl_types =
    composite_t<std::tuple<>, std::conditional_t<Rs::is_direct, void, Rs>...>;

template <class P>
auto weak_lock_impl(const P& self) noexcept
  requires(requires { self.lock(); })
{
  if constexpr (std::is_constructible_v<bool, decltype(self.lock())>) {
    return converter{
        [&self]<class F>(std::in_place_type_t<proxy<F>>) noexcept -> proxy<F> {
          auto strong = self.lock();
          return strong ? proxy<F>{std::move(strong)} : proxy<F>{};
        }};
  } else {
    return self.lock();
  }
}
PRO4_DEF_FREE_AS_MEM_DISPATCH(weak_mem_lock, weak_lock_impl, lock);

template <class O>
struct weak_substitution_overload_traits;
#define PRO4D_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS(oq, pq, ne, ...)           \
  template <class F>                                                           \
  struct weak_substitution_overload_traits<proxy<F>() oq ne>                   \
      : std::type_identity<weak_proxy<F>() oq ne> {};
PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(PRO4D_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS)
#undef PRO4D_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS
template <class... Os>
using weak_substitution_conv =
    conv_impl<true, substitution_dispatch,
              typename weak_substitution_overload_traits<Os>::type...>;

template <class C>
struct weak_conv_traits : std::type_identity<void> {};
template <class C>
  requires(C::is_direct &&
           std::is_same_v<typename C::dispatch_type, substitution_dispatch>)
struct weak_conv_traits<C>
    : std::type_identity<specialization_t<weak_substitution_conv,
                                          typename C::overload_types>> {};
template <class F, class... Cs>
using weak_conv_types = composite_t<
    std::tuple<conv_impl<true, weak_mem_lock, proxy<F>() const noexcept>>,
    typename weak_conv_traits<Cs>::type...>;

} // namespace detail

struct PRO4D_ENFORCE_EBO substitution_dispatch
    : detail::cast_dispatch_base<false, true>,
      detail::internal_dispatch {
  template <class P, class F1>
  PRO4D_STATIC_CALL(auto, std::in_place_type_t<P>, proxy<F1>&& self) noexcept {
    return detail::converter{
        [&self]<class F2>(std::in_place_type_t<proxy<F2>>) noexcept {
          proxy<F2> ret;
          detail::proxy_helper::trivially_relocate<P>(self, ret);
          return ret;
        }};
  }
  template <class T>
  PRO4D_STATIC_CALL(T&&, T&& self) noexcept {
    return std::forward<T>(self);
  }

  // This overload is not reachable at runtime, but is necessary to ensure
  // substitution_dispatch is SFINAE-friendly.
  template <class T>
  PRO4D_STATIC_CALL(auto, T&&) noexcept
    requires(std::is_same_v<T, std::remove_cvref_t<T>> &&
             is_bitwise_trivially_relocatable_v<T>)
  {
    return detail::converter{
        []<class F2>(std::in_place_type_t<proxy<F2>>) noexcept -> proxy<F2>
          requires(proxiable<T, F2>)
        { PRO4D_UNREACHABLE(); }};
  }
};

template <facade F>
struct observer_facade
    : detail::facade_impl<
          detail::specialization_t<detail::observer_conv_types,
                                   typename F::convention_types>,
          detail::specialization_t<detail::observer_refl_types,
                                   typename F::reflection_types>,
          sizeof(void*), alignof(void*), constraint_level::trivial,
          constraint_level::trivial, constraint_level::trivial> {};

template <facade F>
struct weak_facade
    : detail::facade_impl<
          detail::specialization_t<detail::weak_conv_types,
                                   typename F::convention_types, F>,
          std::tuple<>, F::max_size, F::max_align, F::copyability,
          F::relocatability, F::destructibility> {};

} // namespace pro::inline v4

#endif // MSFT_PROXY_V4_DETAIL_CORE_H_
