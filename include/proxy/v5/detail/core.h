// Copyright (c) 2022-2026 Microsoft Corporation.
// Copyright (c) 2026-Present Next Gen C++ Foundation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V5_DETAIL_CORE_H_
#define MSFT_PROXY_V5_DETAIL_CORE_H_

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../proxy_macros.h"
#include "./metadata_policy.h"

#if __has_cpp_attribute(msvc::no_unique_address)
#define PRO5D_NO_UNIQUE_ADDRESS_ATTRIBUTE msvc::no_unique_address
#elif __has_cpp_attribute(no_unique_address)
#define PRO5D_NO_UNIQUE_ADDRESS_ATTRIBUTE no_unique_address
#else
#error Proxy requires C++20 attribute no_unique_address.
#endif // __has_cpp_attribute(msvc::no_unique_address)

#if __cpp_lib_unreachable >= 202202L
#define PRO5D_UNREACHABLE() std::unreachable()
#else
#define PRO5D_UNREACHABLE() std::abort()
#endif // __cpp_lib_unreachable >= 202202L

namespace pro::inline v5 {

namespace detail {

template <class F>
struct basic_facade_traits;
template <class F, class MP>
struct proxy_traits;
template <class F, class MP>
struct proxy_meta;

} // namespace detail

enum class lifetime_requirement {
  none,
  supported,
  nothrow,
  trivial,
  nontrivial [[deprecated("Use lifetime_requirement::supported instead")]] =
      supported
};
using constraint_level [[deprecated("Use lifetime_requirement instead")]] =
    lifetime_requirement;

template <template <class, class> class ST>
struct proxy_dependent_signature {
  proxy_dependent_signature() = delete;
};

template <class D, class S>
struct proxy_operation {};
template <class M>
struct proxy_reflection {};

template <class F>
concept facade = detail::basic_facade_traits<F>::applicable;

template <facade F, class MP = compact_metadata>
class proxy_indirect_accessor;
template <facade F, class MP = compact_metadata>
class PRO5D_ENFORCE_EBO proxy;

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
using specialization_t = specialization_type_traits<T, TL, Args...>::type;

template <class T, template <class...> class TT>
struct specialization_traits : inapplicable_traits {};
template <template <class...> class TT, class... Args>
struct specialization_traits<TT<Args...>, TT> : applicable_traits {};
template <class T, template <class...> class TT>
concept specialization_of = specialization_traits<T, TT>::applicable;

template <template <class, class, class...> class R, class... Args>
struct reduction_t;
template <class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class, class...> class R, class... Args, class O,
          class I, class... Is>
struct recursive_reduction<reduction_t<R, Args...>, O, I, Is...>
    : recursive_reduction<reduction_t<R, Args...>,
                          typename R<O, I, Args...>::type, Is...> {};
template <class R, class O, class... Is>
using recursive_reduction_t = recursive_reduction<R, O, Is...>::type;

template <class O, class I, class R>
struct flattening_reduction : specialization_t<recursive_reduction, I, R, O> {};
template <class R, class... Tss>
struct flattening_merge : std::type_identity<std::tuple<>> {};
template <class R, class Ts, class... Tss>
struct flattening_merge<R, Ts, Tss...>
    : recursive_reduction<reduction_t<flattening_reduction, R>, Ts, Tss...> {};
template <class R, class... Tss>
using flattening_merge_t = flattening_merge<R, Tss...>::type;

template <class O, class I>
struct composition_reduction : std::type_identity<O> {};
template <template <class...> class T, class... Os, class I>
  requires(!std::is_void_v<I>)
struct composition_reduction<T<Os...>, I> : std::type_identity<T<Os..., I>> {};
template <template <class...> class T, class... Os, class... Is>
struct composition_reduction<T<Os...>, T<Is...>>
    : std::type_identity<T<Os..., Is...>> {};
template <class T, class... Us>
using composite_t =
    recursive_reduction_t<reduction_t<composition_reduction>, T, Us...>;

template <class O, class I>
struct add_tuple_reduction : std::type_identity<O> {};
template <class... Os, class I>
  requires(!std::is_same_v<I, Os> && ...)
struct add_tuple_reduction<std::tuple<Os...>, I>
    : std::type_identity<std::tuple<Os..., I>> {};
template <class... Tss>
using merge_tuples_t =
    flattening_merge_t<reduction_t<add_tuple_reduction>, Tss...>;

template <class T, auto V>
  requires(std::is_same_v<T, decltype(V)>)
struct static_prop_probe;

template <class T, std::size_t I>
concept has_tuple_element = requires { typename std::tuple_element_t<I, T>; };
template <class T>
consteval bool is_tuple_like_well_formed() {
  if constexpr (requires {
                  typename static_prop_probe<std::size_t,
                                             std::tuple_size<T>::value>;
                }) {
    return []<std::size_t... I>(std::index_sequence<I...>) {
      return (has_tuple_element<T, I> && ...);
    }(std::make_index_sequence<std::tuple_size_v<T>>{});
  }
  return false;
}

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
using add_qualifier_t = add_qualifier_traits<T, Q>::type;

template <class T, lifetime_requirement LR>
struct copyability_traits : inapplicable_traits {};
template <class T>
struct copyability_traits<T, lifetime_requirement::none> : applicable_traits {};
template <class T>
  requires(std::is_copy_constructible_v<T>)
struct copyability_traits<T, lifetime_requirement::supported>
    : applicable_traits {};
template <class T>
  requires(std::is_nothrow_copy_constructible_v<T>)
struct copyability_traits<T, lifetime_requirement::nothrow>
    : applicable_traits {};
template <class T>
  requires(std::is_trivially_copy_constructible_v<T>)
struct copyability_traits<T, lifetime_requirement::trivial>
    : applicable_traits {};

template <class T, lifetime_requirement LR>
struct relocatability_traits : inapplicable_traits {};
template <class T>
struct relocatability_traits<T, lifetime_requirement::none>
    : applicable_traits {};
template <class T>
  requires((std::is_move_constructible_v<T> && std::is_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, lifetime_requirement::supported>
    : applicable_traits {};
template <class T>
  requires((std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, lifetime_requirement::nothrow>
    : applicable_traits {};
template <class T>
  requires(is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, lifetime_requirement::trivial>
    : applicable_traits {};

template <class T, lifetime_requirement LR>
struct destructibility_traits : inapplicable_traits {};
template <class T>
struct destructibility_traits<T, lifetime_requirement::none>
    : applicable_traits {};
template <class T>
  requires(std::is_destructible_v<T>)
struct destructibility_traits<T, lifetime_requirement::supported>
    : applicable_traits {};
template <class T>
  requires(std::is_nothrow_destructible_v<T>)
struct destructibility_traits<T, lifetime_requirement::nothrow>
    : applicable_traits {};
template <class T>
  requires(std::is_trivially_destructible_v<T>)
struct destructibility_traits<T, lifetime_requirement::trivial>
    : applicable_traits {};

template <class F, class MP, qualifier_type Q>
add_qualifier_t<proxy<F, MP>, Q>
    as_proxy(add_qualifier_t<proxy_indirect_accessor<F, MP>, Q> p);

struct proxy_helper {
  template <class F, class MP>
  struct meta_resetting_guard {
    explicit meta_resetting_guard(proxy<F, MP>& p) noexcept : p_(p) {}
    explicit meta_resetting_guard(proxy_indirect_accessor<F, MP>& p) noexcept
        : p_(as_proxy<F, MP, qualifier_type::lv>(p)) {}
    ~meta_resetting_guard() noexcept { p_.meta_.reset(); }

  private:
    proxy<F, MP>& p_;
  };

  template <class F, class MP>
  static const typename MP::template storage<proxy_meta<F, MP>>&
      get_meta_storage(const proxy<F, MP>& p) noexcept {
    return p.meta_;
  }
  template <class M, class F, class MP>
  static const M& get_meta(const proxy<F, MP>& p) noexcept {
    assert(p.meta_);
    return *p.meta_;
  }
  template <class M, class F, class MP>
  static const M& get_meta(const proxy_indirect_accessor<F, MP>& p) noexcept {
    return get_meta<M>(as_proxy<F, MP, qualifier_type::const_lv>(p));
  }
  template <class F, class MP>
  static void* get_ptr(proxy<F, MP>& p) noexcept {
    return p.ptr_;
  }
  template <class F, class MP>
  static const void* get_ptr(const proxy<F, MP>& p) noexcept {
    return p.ptr_;
  }
  template <class F, class MP>
  static void* get_ptr(proxy_indirect_accessor<F, MP>& p) noexcept {
    return get_ptr(as_proxy<F, MP, qualifier_type::lv>(p));
  }
  template <class F, class MP>
  static const void* get_ptr(const proxy_indirect_accessor<F, MP>& p) noexcept {
    return get_ptr(as_proxy<F, MP, qualifier_type::const_lv>(p));
  }
};

template <class P, bool IsDirect, qualifier_type Q>
struct operand_traits : add_qualifier_traits<P, Q> {};
template <class P, qualifier_type Q>
struct operand_traits<P, false, Q>
    : std::type_identity<decltype(*std::declval<add_qualifier_t<P, Q>>())> {};
template <class P, bool IsDirect, qualifier_type Q>
using operand_t = operand_traits<P, IsDirect, Q>::type;
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

template <class P, bool IsDirect, qualifier_type Q>
operand_t<P, IsDirect, Q>
    get_operand(std::remove_reference_t<add_qualifier_t<P, Q>>* self) {
  if constexpr (IsDirect) {
    return static_cast<add_qualifier_t<P, Q>>(*self);
  } else {
    add_qualifier_t<P, Q> ptr = static_cast<add_qualifier_t<P, Q>>(*self);
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

template <class P>
struct destroying_guard {
  explicit destroying_guard(P* p) noexcept : p_(p) {}
  ~destroying_guard() noexcept(std::is_nothrow_destructible_v<P>) { p_->~P(); }

private:
  P* p_;
};

struct relocate_dispatch;

template <class S>
struct signature_traits : inapplicable_traits {};
template <qualifier_type Q, bool NE, class R, class... Args>
struct signature_traits_impl : applicable_traits {
  using return_type = R;
  using pointer_type = std::conditional_t<Q == qualifier_type::const_lv ||
                                              Q == qualifier_type::const_rv,
                                          const void*, void*>;
  using function_type = R(pointer_type, Args...) noexcept(NE);

  template <class P, bool IsDirect, class D>
  static constexpr auto invoke =
      [](pointer_type ptr, Args... args) noexcept(NE) -> R {
    auto* self = std::launder(
        static_cast<std::remove_reference_t<add_qualifier_t<P, Q>>*>(ptr));
    if constexpr (std::is_same_v<D, relocate_dispatch>) {
      [self](void* rhs) {
        if constexpr (is_bitwise_trivially_relocatable_v<P>) {
          if constexpr (!std::is_empty_v<P>) {
            std::uninitialized_copy_n(reinterpret_cast<const std::byte*>(self),
                                      sizeof(P), static_cast<std::byte*>(rhs));
          }
        } else {
          destroying_guard<P> guard{self};
          std::construct_at(static_cast<P*>(rhs), std::move(*self));
        }
      }(args...);
    } else if constexpr (Q == qualifier_type::rv) {
      destroying_guard<P> guard{self};
      return invoke_dispatch<D, R>(get_operand<P, IsDirect, Q>(self),
                                   std::forward<Args>(args)...);
    } else {
      return invoke_dispatch<D, R>(get_operand<P, IsDirect, Q>(self),
                                   std::forward<Args>(args)...);
    }
  };

  static constexpr qualifier_type this_qualifier = Q;
  template <class P, bool IsDirect, class D>
  static constexpr bool applicable_ptr =
      invocable_dispatch<P, IsDirect, D, Q, NE, R, Args...>;
};
template <class R, class... Args>
struct signature_traits<R(Args...)>
    : signature_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) noexcept>
    : signature_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) &>
    : signature_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) & noexcept>
    : signature_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) &&>
    : signature_traits_impl<qualifier_type::rv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) && noexcept>
    : signature_traits_impl<qualifier_type::rv, true, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const>
    : signature_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const noexcept>
    : signature_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const&>
    : signature_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const & noexcept>
    : signature_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const&&>
    : signature_traits_impl<qualifier_type::const_rv, false, R, Args...> {};
template <class R, class... Args>
struct signature_traits<R(Args...) const && noexcept>
    : signature_traits_impl<qualifier_type::const_rv, true, R, Args...> {};
template <class S>
using ret_t = signature_traits<S>::return_type;

template <class MP, bool IsDirect, class D, class S>
struct invocation_meta {
  invocation_meta() = default;
  template <class P>
  constexpr explicit invocation_meta(std::in_place_type_t<P>)
      : function(signature_traits<S>::template invoke<P, IsDirect, D>) {}

  explicit operator bool() const noexcept {
    return static_cast<bool>(function);
  }

  typename MP::template invoker<typename signature_traits<S>::function_type>
      function;
};

template <class S>
struct signature_substitution_traits : inapplicable_traits {
  template <class, class>
  using type = S;
};
template <template <class, class> class ST>
struct signature_substitution_traits<proxy_dependent_signature<ST>>
    : applicable_traits {
  template <class F, class MP>
  using type = ST<F, MP>;
};
template <class S, class F, class MP>
using substituted_signature_t =
    signature_substitution_traits<S>::template type<F, MP>;
template <class S>
concept extended_signature = signature_traits<S>::applicable ||
                             signature_substitution_traits<S>::applicable;

template <class O>
concept basic_operation = requires {
  { typename O::dispatch_type() } noexcept;
  typename O::signature_type;
  typename static_prop_probe<bool, O::is_direct>;
} && extended_signature<typename O::signature_type>;

template <class M>
concept basic_meta = std::is_nothrow_copy_constructible_v<M> &&
                     std::is_trivially_destructible_v<M>;
template <class M, class T>
concept meta = basic_meta<M> &&
               std::is_nothrow_constructible_v<M, std::in_place_type_t<T>>;

template <class R>
concept basic_reflection = requires {
  typename R::metadata_type;
  typename static_prop_probe<bool, R::is_direct>;
} && basic_meta<typename R::metadata_type>;

template <class T>
concept pointer_like = (std::is_pointer_v<T> ||
    requires { typename T::element_type; } || requires(T val) { *val; }) &&
    requires { typename std::pointer_traits<T>::element_type; };

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
template <class Self, class A, class... Ds>
using accessor_t = a11y_traits<void, A, Self, Ds...>::type;

template <bool IsDirect, class M>
struct reflection_meta {
  template <class P>
    requires(IsDirect)
  constexpr explicit reflection_meta(std::in_place_type_t<P>)
      : metadata(std::in_place_type<P>) {}
  template <class P>
    requires(!IsDirect)
  constexpr explicit reflection_meta(std::in_place_type_t<P>)
      : metadata(
            std::in_place_type<typename std::pointer_traits<P>::element_type>) {
  }

  [[PRO5D_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  M metadata;
};

template <class T, bool IsDirect, class M>
consteval bool is_metadata_well_formed() {
  if constexpr (IsDirect) {
    if constexpr (meta<M, T>) {
      return true;
    }
  } else {
    return is_metadata_well_formed<
        typename std::pointer_traits<T>::element_type, true, M>();
  }
  return false;
}
struct copy_dispatch {
  template <class T>
  PRO5D_STATIC_CALL(void, const T& self, void* rhs) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    std::construct_at(static_cast<T*>(rhs), self);
  }
};
struct destroy_dispatch {
  PRO5D_STATIC_CALL(void, auto&&) noexcept {}
};
template <class MP, class D, class SNE, class SE, lifetime_requirement LR>
struct lifetime_meta_traits : std::type_identity<void> {};
template <class MP, class D, class SNE, class SE>
struct lifetime_meta_traits<MP, D, SNE, SE, lifetime_requirement::nothrow>
    : std::type_identity<invocation_meta<MP, true, D, SNE>> {};
template <class MP, class D, class SNE, class SE>
struct lifetime_meta_traits<MP, D, SNE, SE, lifetime_requirement::supported>
    : std::type_identity<invocation_meta<MP, true, D, SE>> {};
template <class MP, class D, class SNE, class SE, lifetime_requirement LR>
using lifetime_meta_t = lifetime_meta_traits<MP, D, SNE, SE, LR>::type;

template <class... As>
struct PRO5D_ENFORCE_EBO composite_accessor : As... {};

template <class T, class F, class MP>
struct descriptor_substitution_traits : std::type_identity<T> {};
template <class D, class S, class F, class MP>
struct descriptor_substitution_traits<proxy_operation<D, S>, F, MP>
    : std::type_identity<
          proxy_operation<D, substituted_signature_t<S, F, MP>>> {};
template <class T, class F, class MP>
using substituted_descriptor_t = descriptor_substitution_traits<T, F, MP>::type;

template <class A, class... Ds>
struct access_group;
template <class G, class Self, class F, class MP>
struct access_group_accessor_traits;
template <class A, class... Ds, class Self, class F, class MP>
struct access_group_accessor_traits<access_group<A, Ds...>, Self, F, MP>
    : std::type_identity<
          accessor_t<Self, A, substituted_descriptor_t<Ds, F, MP>...>> {};
template <class Self, class F, class MP, class... Gs>
using accessors_t =
    composite_t<composite_accessor<>, typename access_group_accessor_traits<
                                          Gs, Self, F, MP>::type...>;

template <class G, class A>
struct access_group_match_traits : inapplicable_traits {};
template <class A, class... Ds>
struct access_group_match_traits<access_group<A, Ds...>, A>
    : applicable_traits {};

template <class G1, class G2>
struct access_group_merge_traits : std::type_identity<G1> {};
template <class A, class... Ds1, class... Ds2>
struct access_group_merge_traits<access_group<A, Ds1...>,
                                 access_group<A, Ds2...>>
    : specialization_type_traits<
          access_group, merge_tuples_t<std::tuple<Ds1...>, std::tuple<Ds2...>>,
          A> {};

template <class O, class I>
struct access_groups_reduction;
template <class... Gs, class A, class... Ds>
  requires(access_group_match_traits<Gs, A>::applicable || ...)
struct access_groups_reduction<std::tuple<Gs...>, access_group<A, Ds...>>
    : std::type_identity<std::tuple<typename access_group_merge_traits<
          Gs, access_group<A, Ds...>>::type...>> {};
template <class... Gs, class I>
struct access_groups_reduction<std::tuple<Gs...>, I>
    : std::type_identity<std::tuple<Gs..., I>> {};
template <class... Gss>
using access_groups_merge_t =
    flattening_merge_t<reduction_t<access_groups_reduction>, Gss...>;

template <class T>
struct access_traits : std::type_identity<void> {};
template <class T>
  requires(requires { typename T::access_type; })
struct access_traits<T> : std::type_identity<typename T::access_type> {};
template <class T>
using access_t = access_traits<T>::type;

template <class O>
using op_access_group_t =
    access_group<access_t<O>, proxy_operation<typename O::dispatch_type,
                                              typename O::signature_type>>;
template <class R>
using refl_access_group_t =
    access_group<access_t<R>, proxy_reflection<typename R::metadata_type>>;

template <class O, class I, class T>
struct first_containing_reduction : std::type_identity<O> {};
template <class I, class T>
struct first_containing_reduction<void, I, T>
    : std::conditional<std::is_nothrow_convertible_v<const I&, const T&>, I,
                       void> {};

template <class O, class I>
struct most_containing_reduction
    : std::conditional<std::is_nothrow_convertible_v<const I&, const O&>, I,
                       O> {};
template <class SFINAE, class O, class I>
struct sfinae_unique_types_traits : std::type_identity<O> {};
template <class... Ts, class U, class... Us>
struct sfinae_unique_types_traits<
    std::enable_if_t<(std::is_nothrow_convertible_v<const Ts&, const U&> ||
                      ...)>,
    std::tuple<Ts...>, std::tuple<U, Us...>>
    : sfinae_unique_types_traits<void, std::tuple<Ts...>, std::tuple<Us...>> {};
template <class... Ts, class U, class... Us>
struct sfinae_unique_types_traits<
    std::enable_if_t<!(std::is_nothrow_convertible_v<const Ts&, const U&> ||
                       ...)>,
    std::tuple<Ts...>, std::tuple<U, Us...>>
    : sfinae_unique_types_traits<
          void,
          std::tuple<Ts...,
                     recursive_reduction_t<
                         reduction_t<most_containing_reduction>, U, Us...>>,
          std::tuple<Us...>> {};
template <class T>
using unique_types_t = sfinae_unique_types_traits<void, std::tuple<>, T>::type;

struct sentinel_meta {
  sentinel_meta() = default;
  template <class P>
  constexpr explicit sentinel_meta(std::in_place_type_t<P>) noexcept
      : v_(true) {}
  explicit operator bool() const noexcept { return v_; }

private:
  bool v_;
};

template <class... Ms>
struct PRO5D_ENFORCE_EBO meta_pack : Ms... {
  template <class P>
  constexpr explicit meta_pack(std::in_place_type_t<P>)
      : Ms(std::in_place_type<P>)... {}
};

template <class First, class... Rest>
class proxy_meta_base_impl {
public:
  constexpr proxy_meta_base_impl() noexcept : first_() {}
  template <class P>
  constexpr explicit proxy_meta_base_impl(std::in_place_type_t<P>)
      : first_(std::in_place_type<P>), rest_(std::in_place_type<P>) {}
  proxy_meta_base_impl(const proxy_meta_base_impl& rhs) noexcept
      : first_(rhs.first_) {
    if (rhs) {
      std::construct_at(std::addressof(rest_), rhs.rest_);
    }
  }
  proxy_meta_base_impl& operator=(const proxy_meta_base_impl& rhs) noexcept {
    if (this != std::addressof(rhs)) {
      first_ = rhs.first_;
      if (rhs) {
        std::construct_at(std::addressof(rest_), rhs.rest_);
      }
    }
    return *this;
  }

  template <class T>
    requires(std::is_nothrow_convertible_v<const First&, const T&> ||
             (std::is_nothrow_convertible_v<const Rest&, const T&> || ...))
  constexpr operator const T&() const noexcept {
    using M = recursive_reduction_t<reduction_t<first_containing_reduction, T>,
                                    void, First, Rest...>;
    if constexpr (std::is_same_v<M, First>) {
      return first_;
    } else {
      return static_cast<const M&>(rest_);
    }
  }
  explicit operator bool() const noexcept { return static_cast<bool>(first_); }

private:
  First first_;
  union {
    meta_pack<Rest...> rest_;
  };
};
template <class First>
class proxy_meta_base_impl<First> : public First {
public:
  constexpr proxy_meta_base_impl() noexcept : First() {}
  using First::First;
};

template <class... Ms>
struct proxy_meta_base_traits
    : specialization_type_traits<proxy_meta_base_impl,
                                 unique_types_t<std::tuple<Ms...>>,
                                 sentinel_meta> {};
template <class MP, bool IsDirect, class D, class S, class... Ms>
struct proxy_meta_base_traits<invocation_meta<MP, IsDirect, D, S>, Ms...>
    : specialization_type_traits<
          proxy_meta_base_impl,
          unique_types_t<
              std::tuple<invocation_meta<MP, IsDirect, D, S>, Ms...>>> {};
template <class F, class MP, class... Ms>
struct proxy_meta_base_traits<proxy_meta<F, MP>, Ms...>
    : specialization_type_traits<
          proxy_meta_base_impl,
          unique_types_t<std::tuple<proxy_meta<F, MP>, Ms...>>> {};
template <class... Ms>
using proxy_meta_base_t = typename proxy_meta_base_traits<Ms...>::type;

template <class P, class F, std::size_t ActualSize, std::size_t MaxSize>
consteval void diagnose_proxiable_size_too_large() {
  static_assert(ActualSize <= MaxSize, "not proxiable due to size too large");
}
template <class P, class F, std::size_t ActualAlign, std::size_t MaxAlign>
consteval void diagnose_proxiable_align_too_large() {
  static_assert(ActualAlign <= MaxAlign,
                "not proxiable due to alignment too large");
}
template <class P, class F, lifetime_requirement RequiredCopyability>
consteval void diagnose_proxiable_insufficient_copyability() {
  static_assert(copyability_traits<P, RequiredCopyability>::applicable,
                "not proxiable due to insufficient copyability");
}
template <class P, class F, lifetime_requirement RequiredRelocatability>
consteval void diagnose_proxiable_insufficient_relocatability() {
  static_assert(relocatability_traits<P, RequiredRelocatability>::applicable,
                "not proxiable due to insufficient relocatability");
}
template <class P, class F, lifetime_requirement RequiredDestructibility>
consteval void diagnose_proxiable_insufficient_destructibility() {
  static_assert(destructibility_traits<P, RequiredDestructibility>::applicable,
                "not proxiable due to insufficient destructibility");
}
template <class P, class F, class MP, bool IsDirect, class D, class S>
consteval void diagnose_proxiable_required_operation_not_implemented() {
  static_assert(signature_traits<substituted_signature_t<S, F, MP>>::
                    template applicable_ptr<P, IsDirect, D>,
                "not proxiable due to a required operation not implemented");
}
template <class P, class F, bool IsDirect, class M>
consteval void diagnose_proxiable_required_reflection_not_implemented() {
  static_assert(is_metadata_well_formed<P, IsDirect, M>(),
                "not proxiable due to a required reflection not implemented");
}

consteval bool is_layout_well_formed(std::size_t size, std::size_t align) {
  return size > 0u && std::has_single_bit(align) && size % align == 0u;
}
consteval bool is_lr_well_formed(lifetime_requirement lr) {
  return lr >= lifetime_requirement::none &&
         lr <= lifetime_requirement::trivial;
}
template <class F>
consteval bool is_facade_constraints_well_formed() {
  if constexpr (requires {
                  typename static_prop_probe<std::size_t, F::max_size>;
                  typename static_prop_probe<std::size_t, F::max_align>;
                  typename static_prop_probe<lifetime_requirement,
                                             F::copyability>;
                  typename static_prop_probe<lifetime_requirement,
                                             F::relocatability>;
                  typename static_prop_probe<lifetime_requirement,
                                             F::destructibility>;
                }) {
    return is_layout_well_formed(F::max_size, F::max_align) &&
           is_lr_well_formed(F::copyability) &&
           is_lr_well_formed(F::relocatability) &&
           is_lr_well_formed(F::destructibility);
  }
  return false;
}
template <class MP>
consteval bool is_metadata_policy_well_formed() {
  using F = void(void*) noexcept;
  using M = proxy_meta_base_t<>;
  if constexpr (requires {
                  typename MP::template invoker<F>;
                  typename MP::template storage<M>;
                }) {
    using I = typename MP::template invoker<F>;
    using S = typename MP::template storage<M>;
    using C = decltype([](void*) noexcept {});
    return requires(const I& ci, void* p, S& s, const S& cs) {
      { static_cast<bool>(ci) } noexcept;
      { ci(p) } noexcept -> std::same_as<void>;
      { s.reset() } noexcept;
      { s.emplace(std::in_place_type<void*>) } noexcept;
      { static_cast<bool>(cs) } noexcept;
      { *cs } noexcept -> std::same_as<const M&>;
    } && std::is_nothrow_default_constructible_v<I> &&
           std::is_nothrow_constructible_v<I, const C&> &&
           std::is_nothrow_copy_assignable_v<I> && basic_meta<I> &&
           std::is_nothrow_default_constructible_v<S> &&
           std::is_nothrow_copy_constructible_v<S> &&
           std::is_nothrow_copy_assignable_v<S> &&
           std::is_nothrow_destructible_v<S>;
  }
  return false;
}
template <class F, class G>
consteval bool is_super_constraints_well_formed() {
  return F::max_size <= G::max_size && F::max_align <= G::max_align &&
         F::copyability >= G::copyability &&
         F::relocatability >= G::relocatability &&
         F::destructibility >= G::destructibility;
}
template <class F, class... Fs>
struct basic_facade_super_traits_impl : inapplicable_traits {};
template <class F, class... Fs>
  requires((facade<Fs> && ...) &&
           (is_super_constraints_well_formed<F, Fs>() && ...))
struct basic_facade_super_traits_impl<F, Fs...> : applicable_traits {};
template <class... Os>
struct basic_facade_op_traits_impl : inapplicable_traits {};
template <class... Os>
  requires(basic_operation<Os> && ...)
struct basic_facade_op_traits_impl<Os...> : applicable_traits {};
template <class... Rs>
struct basic_facade_refl_traits_impl : inapplicable_traits {};
template <class... Rs>
  requires(basic_reflection<Rs> && ...)
struct basic_facade_refl_traits_impl<Rs...> : applicable_traits {};
template <class F>
struct basic_facade_traits : inapplicable_traits {};
template <class F>
  requires(requires {
    typename F::super_types;
    typename F::operation_types;
    typename F::reflection_types;
  } && is_facade_constraints_well_formed<F>() &&
           is_tuple_like_well_formed<typename F::super_types>() &&
           specialization_t<basic_facade_super_traits_impl,
                            typename F::super_types, F>::applicable &&
           is_tuple_like_well_formed<typename F::operation_types>() &&
           specialization_t<basic_facade_op_traits_impl,
                            typename F::operation_types>::applicable &&
           is_tuple_like_well_formed<typename F::reflection_types>() &&
           specialization_t<basic_facade_refl_traits_impl,
                            typename F::reflection_types>::applicable)
struct basic_facade_traits<F> : applicable_traits {};

template <class F, class MP, class... Os>
struct op_traits_impl {
  static_assert((signature_traits<substituted_signature_t<
                     typename Os::signature_type, F, MP>>::applicable &&
                 ...),
                "a proxy-dependent signature did not substitute into a valid "
                "signature");
  using ops = std::tuple<Os...>;
  using op_meta = std::tuple<invocation_meta<
      MP, Os::is_direct, typename Os::dispatch_type,
      substituted_signature_t<typename Os::signature_type, F, MP>>...>;

  template <class P>
  static consteval void diagnose_proxiable_op() {
    (diagnose_proxiable_required_operation_not_implemented<
         P, F, MP, Os::is_direct, typename Os::dispatch_type,
         typename Os::signature_type>(),
     ...);
  }

  template <class P>
  static constexpr bool op_applicable_ptr =
      (signature_traits<
           substituted_signature_t<typename Os::signature_type, F, MP>>::
           template applicable_ptr<P, Os::is_direct,
                                   typename Os::dispatch_type> &&
       ...);
};
template <class F, class MP, class... Fs>
struct proxy_super_traits_impl
    : specialization_t<
          op_traits_impl,
          merge_tuples_t<typename proxy_traits<Fs, MP>::dependent_ops...>, F,
          MP> {
  using super_dependent_ops = typename proxy_super_traits_impl::ops;
  using super_indirect_access_groups = access_groups_merge_t<
      typename proxy_traits<Fs, MP>::indirect_access_groups...>;
  using super_direct_access_groups = access_groups_merge_t<
      typename proxy_traits<Fs, MP>::direct_access_groups...>;
  using super_meta = composite_t<std::tuple<proxy_meta<Fs, MP>...>,
                                 typename proxy_super_traits_impl::op_meta>;

  template <class P>
  static consteval void diagnose_proxiable_super() {
    (proxy_traits<Fs, MP>::template diagnose_proxiable<P>(), ...);
    proxy_super_traits_impl::template diagnose_proxiable_op<P>();
  }

  template <class P>
  static constexpr bool super_applicable_ptr =
      (proxy_traits<Fs, MP>::template applicable_ptr<P> && ...) &&
      proxy_super_traits_impl::template op_applicable_ptr<P>;
};
template <class F, class MP, class... Os>
struct proxy_op_traits_impl : op_traits_impl<F, MP, Os...> {
  using self_op_meta = typename proxy_op_traits_impl::op_meta;
  using self_indirect_op_groups =
      composite_t<std::tuple<>, std::conditional_t<Os::is_direct, void,
                                                   op_access_group_t<Os>>...>;
  using self_direct_op_groups = composite_t<
      std::tuple<>,
      std::conditional_t<Os::is_direct, op_access_group_t<Os>, void>...>;
  using self_dependent_ops = composite_t<
      std::tuple<>,
      std::conditional_t<signature_substitution_traits<
                             typename Os::signature_type>::applicable,
                         Os, void>...>;

  template <class P>
  static consteval void diagnose_proxiable_self_op() {
    proxy_op_traits_impl::template diagnose_proxiable_op<P>();
  }

  template <class P>
  static constexpr bool self_op_applicable_ptr =
      proxy_op_traits_impl::template op_applicable_ptr<P>;
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using self_indirect_refl_groups =
      composite_t<std::tuple<>, std::conditional_t<Rs::is_direct, void,
                                                   refl_access_group_t<Rs>>...>;
  using self_direct_refl_groups = composite_t<
      std::tuple<>,
      std::conditional_t<Rs::is_direct, refl_access_group_t<Rs>, void>...>;
  using refl_meta =
      std::tuple<reflection_meta<Rs::is_direct, typename Rs::metadata_type>...>;

  template <class P>
  static consteval void diagnose_proxiable_refl() {
    (diagnose_proxiable_required_reflection_not_implemented<
         P, F, Rs::is_direct, typename Rs::metadata_type>(),
     ...);
  }

  template <class P>
  static constexpr bool refl_applicable_ptr =
      (is_metadata_well_formed<P, Rs::is_direct,
                               typename Rs::metadata_type>() &&
       ...);
};
template <class F, class MP>
struct proxy_traits
    : specialization_t<proxy_super_traits_impl, typename F::super_types, F, MP>,
      specialization_t<proxy_op_traits_impl, typename F::operation_types, F,
                       MP>,
      specialization_t<facade_refl_traits_impl, typename F::reflection_types,
                       F> {
  using indirect_access_groups =
      access_groups_merge_t<typename proxy_traits::super_indirect_access_groups,
                            typename proxy_traits::self_indirect_op_groups,
                            typename proxy_traits::self_indirect_refl_groups>;
  using direct_access_groups =
      access_groups_merge_t<typename proxy_traits::super_direct_access_groups,
                            typename proxy_traits::self_direct_op_groups,
                            typename proxy_traits::self_direct_refl_groups>;
  using dependent_ops =
      merge_tuples_t<typename proxy_traits::super_dependent_ops,
                     typename proxy_traits::self_dependent_ops>;
  using indirect_accessor =
      specialization_t<accessors_t, indirect_access_groups,
                       proxy_indirect_accessor<F, MP>, F, MP>;
  using direct_accessor =
      specialization_t<accessors_t, direct_access_groups, proxy<F, MP>, F, MP>;
  using meta_base = specialization_t<
      proxy_meta_base_t,
      composite_t<
          typename proxy_traits::super_meta,
          lifetime_meta_t<MP, copy_dispatch, void(void*) const noexcept,
                          void(void*) const, F::copyability>,
          lifetime_meta_t<MP, relocate_dispatch, void(void*) && noexcept,
                          void(void*) &&, F::relocatability>,
          lifetime_meta_t<MP, destroy_dispatch, void() && noexcept, void() &&,
                          F::destructibility>,
          typename proxy_traits::self_op_meta,
          typename proxy_traits::refl_meta>>;

  template <class P>
  static consteval void diagnose_proxiable() {
    diagnose_proxiable_size_too_large<P, F, sizeof(P), F::max_size>();
    diagnose_proxiable_align_too_large<P, F, alignof(P), F::max_align>();
    diagnose_proxiable_insufficient_copyability<P, F, F::copyability>();
    diagnose_proxiable_insufficient_relocatability<P, F, F::relocatability>();
    diagnose_proxiable_insufficient_destructibility<P, F, F::destructibility>();
    proxy_traits::template diagnose_proxiable_super<P>();
    proxy_traits::template diagnose_proxiable_self_op<P>();
    proxy_traits::template diagnose_proxiable_refl<P>();
  }

  template <class P>
  [[noreturn]] static consteval void diagnose_proxiable_noreturn() {
    diagnose_proxiable<P>();
    PRO5D_UNREACHABLE(); // Propagate the error to the caller side
  }

  template <class P>
  static constexpr bool applicable_ptr =
      sizeof(P) <= F::max_size && alignof(P) <= F::max_align &&
      copyability_traits<P, F::copyability>::applicable &&
      relocatability_traits<P, F::relocatability>::applicable &&
      destructibility_traits<P, F::destructibility>::applicable &&
      proxy_traits::template super_applicable_ptr<P> &&
      proxy_traits::template self_op_applicable_ptr<P> &&
      proxy_traits::template refl_applicable_ptr<P>;
};

template <class F, class MP>
struct proxy_meta : proxy_traits<F, MP>::meta_base {
  using base = proxy_traits<F, MP>::meta_base;

  constexpr proxy_meta() noexcept {}
  using base::base;
};

template <class T>
class inplace_ptr {
public:
  template <class Ignore, class... Args>
  explicit inplace_ptr(const Ignore&, Args&&... args)
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
  [[PRO5D_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  T value_;
};

template <class F, class MP, qualifier_type Q>
add_qualifier_t<proxy<F, MP>, Q>
    as_proxy(add_qualifier_t<proxy_indirect_accessor<F, MP>, Q> p) {
  return static_cast<add_qualifier_t<proxy<F, MP>, Q>>(
      reinterpret_cast<
          add_qualifier_t<inplace_ptr<proxy_indirect_accessor<F, MP>>, Q>>(p));
}
template <class F, class MP, bool IsDirect, class D, class S, class P,
          class... Args>
ret_t<S> invoke_impl(P&& p, Args&&... args) {
  auto* ptr = proxy_helper::get_ptr(p);
  auto& inv = proxy_helper::get_meta<invocation_meta<MP, IsDirect, D, S>>(p);
  if constexpr (signature_traits<S>::this_qualifier == qualifier_type::rv) {
    proxy_helper::meta_resetting_guard<F, MP> guard{p};
    return inv.function(ptr, std::forward<Args>(args)...);
  } else {
    return inv.function(ptr, std::forward<Args>(args)...);
  }
}

} // namespace detail

template <class P, class F, class MP = compact_metadata>
concept proxiable = facade<F> && detail::is_metadata_policy_well_formed<MP>() &&
                    detail::pointer_like<P> &&
                    detail::proxy_traits<F, MP>::template applicable_ptr<P>;

template <facade F, class MP>
class proxy_indirect_accessor
    : public detail::proxy_traits<F, MP>::indirect_accessor {
  friend class detail::inplace_ptr<proxy_indirect_accessor>;
  proxy_indirect_accessor() = default;
  proxy_indirect_accessor(const proxy_indirect_accessor&) = default;
  proxy_indirect_accessor& operator=(const proxy_indirect_accessor&) = default;

public:
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(proxy_indirect_accessor& p, Args&&... args) {
    return detail::invoke_impl<F, MP, false, D, S>(p,
                                                   std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(const proxy_indirect_accessor& p,
                                 Args&&... args) {
    return detail::invoke_impl<F, MP, false, D, S>(p,
                                                   std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(proxy_indirect_accessor&& p, Args&&... args) {
    return detail::invoke_impl<F, MP, false, D, S>(std::move(p),
                                                   std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(const proxy_indirect_accessor&& p,
                                 Args&&... args) {
    return detail::invoke_impl<F, MP, false, D, S>(std::move(p),
                                                   std::forward<Args>(args)...);
  }
  template <class M>
  friend const M& reflect(const proxy_indirect_accessor& p) noexcept {
    return detail::proxy_helper::get_meta<detail::reflection_meta<false, M>>(p)
        .metadata;
  }
};

template <facade F, class MP>
class proxy : public detail::proxy_traits<F, MP>::direct_accessor,
              public detail::inplace_ptr<proxy_indirect_accessor<F, MP>> {
  static_assert(detail::is_metadata_policy_well_formed<MP>(),
                "the metadata policy is not well-formed");

  friend struct detail::proxy_helper;

public:
  using facade_type = F;
  using metadata_policy_type = MP;

  proxy() noexcept { initialize(); }
  proxy(std::nullptr_t) noexcept { initialize(); }
  proxy(const proxy&) noexcept
    requires(F::copyability == lifetime_requirement::trivial)
  = default;
  proxy(const proxy& rhs) noexcept(F::copyability ==
                                   lifetime_requirement::nothrow)
    requires(F::copyability == lifetime_requirement::supported ||
             F::copyability == lifetime_requirement::nothrow)
      : detail::inplace_ptr<
            proxy_indirect_accessor<F, MP>>() /* Make GCC happy */ {
    initialize(rhs);
  }
  proxy(proxy&& rhs) noexcept(F::relocatability >=
                              lifetime_requirement::nothrow)
    requires(F::relocatability >= lifetime_requirement::supported &&
             F::copyability != lifetime_requirement::trivial)
  {
    initialize(std::move(rhs));
  }
  template <facade F2>
  proxy(const proxy<F2, MP>& rhs) noexcept(F::copyability >=
                                           lifetime_requirement::nothrow)
    requires(!std::is_same_v<F, F2> &&
             std::is_convertible_v<const detail::proxy_meta<F2, MP>&,
                                   const detail::proxy_meta<F, MP>&> &&
             F::copyability >= lifetime_requirement::supported)
      : detail::inplace_ptr<proxy_indirect_accessor<F, MP>>() {
    initialize(rhs);
  }
  template <facade F2>
  proxy(proxy<F2, MP>&& rhs) noexcept(F::relocatability >=
                                      lifetime_requirement::nothrow)
    requires(!std::is_same_v<F, F2> &&
             std::is_convertible_v<const detail::proxy_meta<F2, MP>&,
                                   const detail::proxy_meta<F, MP>&> &&
             F::relocatability >= lifetime_requirement::supported &&
             F::copyability != lifetime_requirement::trivial)
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
                                            lifetime_requirement::nothrow)
    requires(F::destructibility >= lifetime_requirement::supported)
  {
    reset();
    return *this;
  }
  proxy& operator=(const proxy&) noexcept
    requires(F::copyability == lifetime_requirement::trivial)
  = default;
  proxy& operator=(const proxy& rhs) noexcept(
      F::copyability >= lifetime_requirement::nothrow &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires((F::copyability == lifetime_requirement::supported ||
              F::copyability == lifetime_requirement::nothrow) &&
             F::destructibility >= lifetime_requirement::supported)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      if constexpr (F::copyability == lifetime_requirement::nothrow) {
        destroy();
        initialize(rhs);
      } else if constexpr (F::relocatability >=
                           lifetime_requirement::supported) {
        *this = proxy{rhs};
      } else {
        reset();
        initialize(rhs);
      }
    }
    return *this;
  }
  proxy& operator=(proxy&& rhs) noexcept(F::relocatability >=
                                             lifetime_requirement::nothrow &&
                                         F::destructibility >=
                                             lifetime_requirement::nothrow)
    requires(F::relocatability >= lifetime_requirement::supported &&
             F::destructibility >= lifetime_requirement::supported &&
             F::copyability != lifetime_requirement::trivial)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      reset();
      initialize(std::move(rhs));
    }
    return *this;
  }
  template <facade F2>
  proxy& operator=(const proxy<F2, MP>& rhs) noexcept(
      F::copyability >= lifetime_requirement::nothrow &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires(!std::is_same_v<F, F2> &&
             std::is_convertible_v<const detail::proxy_meta<F2, MP>&,
                                   const detail::proxy_meta<F, MP>&> &&
             F::copyability >= lifetime_requirement::supported &&
             F::destructibility >= lifetime_requirement::supported)
  {
    if constexpr (F::copyability >= lifetime_requirement::nothrow) {
      destroy();
      initialize(rhs);
    } else if constexpr (F::relocatability >= lifetime_requirement::supported) {
      *this = proxy{rhs};
    } else {
      reset();
      initialize(rhs);
    }
    return *this;
  }
  template <facade F2>
  proxy& operator=(proxy<F2, MP>&& rhs) noexcept(
      F::relocatability >= lifetime_requirement::nothrow &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires(!std::is_same_v<F, F2> &&
             std::is_convertible_v<const detail::proxy_meta<F2, MP>&,
                                   const detail::proxy_meta<F, MP>&> &&
             F::relocatability >= lifetime_requirement::supported &&
             F::destructibility >= lifetime_requirement::supported &&
             F::copyability != lifetime_requirement::trivial)
  {
    reset();
    initialize(std::move(rhs));
    return *this;
  }
  template <class P>
  constexpr proxy& operator=(P&& ptr) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<P>, P> &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires(!detail::specialization_of<std::decay_t<P>, proxy> &&
             detail::pointer_like<std::decay_t<P>> &&
             std::is_constructible_v<std::decay_t<P>, P> &&
             F::destructibility >= lifetime_requirement::supported)
  {
    if constexpr (std::is_nothrow_constructible_v<std::decay_t<P>, P>) {
      destroy();
      initialize<std::decay_t<P>>(std::forward<P>(ptr));
    } else if constexpr (F::relocatability >= lifetime_requirement::supported ||
                         F::copyability >= lifetime_requirement::nothrow) {
      *this = proxy{std::forward<P>(ptr)};
    } else {
      reset();
      initialize<std::decay_t<P>>(std::forward<P>(ptr));
    }
    return *this;
  }
  ~proxy()
    requires(F::destructibility == lifetime_requirement::none)
  = delete;
  ~proxy()
    requires(F::destructibility == lifetime_requirement::trivial)
  = default;
  ~proxy() noexcept(F::destructibility == lifetime_requirement::nothrow)
    requires(F::destructibility == lifetime_requirement::supported ||
             F::destructibility == lifetime_requirement::nothrow)
  {
    destroy();
  }

  bool has_value() const noexcept { return static_cast<bool>(meta_); }
  explicit operator bool() const noexcept { return static_cast<bool>(meta_); }
  void reset() noexcept(F::destructibility >= lifetime_requirement::nothrow)
    requires(F::destructibility >= lifetime_requirement::supported)
  {
    destroy();
    initialize();
  }
  void swap(proxy& rhs) noexcept(F::relocatability >=
                                     lifetime_requirement::nothrow ||
                                 F::copyability ==
                                     lifetime_requirement::trivial)
    requires(F::relocatability == lifetime_requirement::trivial ||
             F::copyability == lifetime_requirement::trivial ||
             (F::relocatability >= lifetime_requirement::supported &&
              F::destructibility >= lifetime_requirement::supported))
  {
    if constexpr (F::relocatability == lifetime_requirement::trivial ||
                  F::copyability == lifetime_requirement::trivial) {
      auto meta = std::as_const(meta_);
      meta_ = std::as_const(rhs.meta_);
      rhs.meta_ = std::as_const(meta);
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
      if (meta_) {
        if (rhs.meta_) {
          proxy temp = std::move(*this);
          initialize(std::move(rhs));
          rhs.initialize(std::move(temp));
        } else {
          rhs.initialize(std::move(*this));
        }
      } else if (rhs.meta_) {
        initialize(std::move(rhs));
      }
    }
  }
  void swap(proxy& rhs) noexcept(F::copyability >=
                                     lifetime_requirement::nothrow &&
                                 F::destructibility >=
                                     lifetime_requirement::nothrow)
    requires(F::relocatability == lifetime_requirement::none &&
             (F::copyability == lifetime_requirement::supported ||
              F::copyability == lifetime_requirement::nothrow) &&
             F::destructibility >= lifetime_requirement::supported)
  {
    if (meta_) {
      if (rhs.meta_) {
        proxy temp = *this;
        *this = rhs;
        rhs = temp;
      } else {
        rhs = *this;
        reset();
      }
    } else if (rhs.meta_) {
      *this = rhs;
      rhs.reset();
    }
  }
  template <class P, class... Args>
  constexpr P& emplace(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, Args...> &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires(detail::pointer_like<P> && std::is_constructible_v<P, Args...> &&
             F::destructibility >= lifetime_requirement::supported)
  {
    reset();
    return initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  constexpr P& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, std::initializer_list<U>&, Args...> &&
      F::destructibility >= lifetime_requirement::nothrow)
    requires(detail::pointer_like<P> &&
             std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
             F::destructibility >= lifetime_requirement::supported)
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
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(proxy& p, Args&&... args) {
    return detail::invoke_impl<F, MP, true, D, S>(p,
                                                  std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(const proxy& p, Args&&... args) {
    return detail::invoke_impl<F, MP, true, D, S>(p,
                                                  std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(proxy&& p, Args&&... args) {
    return detail::invoke_impl<F, MP, true, D, S>(std::move(p),
                                                  std::forward<Args>(args)...);
  }
  template <class D, class S, class... Args>
  friend detail::ret_t<S> invoke(const proxy&& p, Args&&... args) {
    return detail::invoke_impl<F, MP, true, D, S>(std::move(p),
                                                  std::forward<Args>(args)...);
  }
  template <class M>
  friend const M& reflect(const proxy& p) noexcept {
    return detail::proxy_helper::get_meta<detail::reflection_meta<true, M>>(p)
        .metadata;
  }

private:
  void initialize() {
    PRO5D_DEBUG(std::ignore = &pro_symbol_guard;)
    meta_.reset();
  }
  template <facade F2>
  void initialize(const proxy<F2, MP>& rhs) {
    PRO5D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.has_value()) {
      if constexpr (F2::copyability == lifetime_requirement::trivial) {
        std::uninitialized_copy_n(
            static_cast<const std::byte*>(detail::proxy_helper::get_ptr(rhs)),
            F2::max_size, ptr_);
      } else {
        invoke<detail::copy_dispatch,
               void(void*) const noexcept(
                   F::copyability == lifetime_requirement::nothrow)>(rhs, ptr_);
      }
      meta_ = detail::proxy_helper::get_meta_storage(rhs);
    } else {
      meta_.reset();
    }
  }
  template <facade F2>
  void initialize(proxy<F2, MP>&& rhs) {
    PRO5D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.has_value()) {
      detail::proxy_helper::meta_resetting_guard<F2, MP> guard{rhs};
      void* rhs_ptr = detail::proxy_helper::get_ptr(rhs);
      if constexpr (F2::relocatability == lifetime_requirement::trivial) {
        std::uninitialized_copy_n(static_cast<std::byte*>(rhs_ptr),
                                  F2::max_size, ptr_);
      } else {
        detail::proxy_helper::get_meta<detail::invocation_meta<
            MP, true, detail::relocate_dispatch,
            void(void*) && noexcept(F::relocatability ==
                                    lifetime_requirement::nothrow)>>(rhs)
            .function(rhs_ptr, ptr_);
      }
      meta_ = detail::proxy_helper::get_meta_storage(rhs);
    } else {
      meta_.reset();
    }
  }
  template <class P, class... Args>
  constexpr P& initialize(Args&&... args) {
    PRO5D_DEBUG(std::ignore = &pro_symbol_guard;)
    P& result = *std::construct_at(reinterpret_cast<P*>(ptr_),
                                   std::forward<Args>(args)...);
    if constexpr (proxiable<P, F, MP>) {
      meta_.emplace(std::in_place_type<P>);
    } else {
      detail::proxy_traits<F, MP>::template diagnose_proxiable_noreturn<P>();
    }
    return result;
  }
  void destroy()
    requires(F::destructibility != lifetime_requirement::none)
  {
    if constexpr (F::destructibility != lifetime_requirement::trivial) {
      if (meta_) {
        invoke<detail::destroy_dispatch,
               void() && noexcept(F::destructibility ==
                                  lifetime_requirement::nothrow)>(
            std::move(*this));
      }
    }
  }
  PRO5D_DEBUG(static inline void pro_symbol_guard(proxy& self,
                                                  const proxy& cself) {
    self.operator->();
    *self;
    *std::move(self);
    cself.operator->();
    *cself;
    *std::move(cself);
  })

  alignas(F::max_align) std::byte ptr_[F::max_size];
  typename MP::template storage<detail::proxy_meta<F, MP>> meta_;
};

template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(proxy_indirect_accessor<F, MP>& p, Args&&... args) {
  return invoke<D, S>(p, std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(const proxy_indirect_accessor<F, MP>& p, Args&&... args) {
  return invoke<D, S>(p, std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(proxy_indirect_accessor<F, MP>&& p, Args&&... args) {
  return invoke<D, S>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(const proxy_indirect_accessor<F, MP>&& p, Args&&... args) {
  return invoke<D, S>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(proxy<F, MP>& p, Args&&... args) {
  return invoke<D, S>(p, std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(const proxy<F, MP>& p, Args&&... args) {
  return invoke<D, S>(p, std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(proxy<F, MP>&& p, Args&&... args) {
  return invoke<D, S>(std::move(p), std::forward<Args>(args)...);
}
template <class D, class S, facade F, class MP, class... Args>
[[deprecated("Use unqualified invoke instead")]] detail::ret_t<S>
    proxy_invoke(const proxy<F, MP>&& p, Args&&... args) {
  return invoke<D, S>(std::move(p), std::forward<Args>(args)...);
}

template <class M, facade F, class MP>
[[deprecated("Use unqualified reflect instead")]] const M&
    proxy_reflect(const proxy_indirect_accessor<F, MP>& p) noexcept {
  return reflect<M>(p);
}
template <class M, facade F, class MP>
[[deprecated("Use unqualified reflect instead")]] const M&
    proxy_reflect(const proxy<F, MP>& p) noexcept {
  return reflect<M>(p);
}

template <facade F>
struct view_facade;
template <facade F, class MP = compact_metadata>
using proxy_view = proxy<view_facade<F>, MP>;

template <facade F>
struct weak_facade;
template <facade F, class MP = compact_metadata>
using weak_proxy = proxy<weak_facade<F>, MP>;

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
             !std::is_invocable_r_v<T, F, std::in_place_type_t<T&&>>)
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
    requires(std::is_invocable_r_v<T&&, F, std::in_place_type_t<T&&>>)
  {
    return std::move(f_)(std::in_place_type<T&&>);
  }

private:
  F f_;
};

template <bool IsDirect, class D, class S, class A>
struct op_impl {
  static constexpr bool is_direct = IsDirect;
  using dispatch_type = D;
  using signature_type = S;
  using access_type = A;
};
template <bool IsDirect, class M, class A>
struct refl_impl {
  static constexpr bool is_direct = IsDirect;
  using metadata_type = M;
  using access_type = A;
};
template <class Self, class... Ds>
struct legacy_accessor_traits {};
template <class Self, class D, class... Ss>
  requires(requires { typename D::template accessor<Self, D, Ss...>; })
struct legacy_accessor_traits<Self, proxy_operation<D, Ss>...>
    : std::type_identity<typename D::template accessor<Self, D, Ss...>> {};
template <class Self, class M>
  requires(requires { typename M::template accessor<Self, M>; })
struct legacy_accessor_traits<Self, proxy_reflection<M>>
    : std::type_identity<typename M::template accessor<Self, M>> {};
template <class T>
struct legacy_access {
  template <class Self, class... Ds>
  using accessor = typename legacy_accessor_traits<Self, Ds...>::type;
};
template <class T>
struct default_access_traits : std::type_identity<legacy_access<T>> {};
template <class T>
using default_access_t = default_access_traits<T>::type;

template <class Fs, class Os, class Rs, std::size_t MaxSize,
          std::size_t MaxAlign, lifetime_requirement Copyability,
          lifetime_requirement Relocatability,
          lifetime_requirement Destructibility>
struct facade_impl {
  using super_types = Fs;
  using operation_types = Os;
  using reflection_types = Rs;
  static constexpr std::size_t max_size = MaxSize;
  static constexpr std::size_t max_align = MaxAlign;
  static constexpr lifetime_requirement copyability = Copyability;
  static constexpr lifetime_requirement relocatability = Relocatability;
  static constexpr lifetime_requirement destructibility = Destructibility;
};

template <class LR, class CLR, class RR, class CRR>
class observer_ptr {
public:
  explicit observer_ptr(LR lr) noexcept : lr_(lr) {}
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
struct view_op_traits : std::type_identity<void> {};
template <class O>
  requires(!O::is_direct)
struct view_op_traits<O> : std::type_identity<O> {};
template <class... Os>
using view_op_types =
    composite_t<std::tuple<>, typename view_op_traits<Os>::type...>;
template <class... Rs>
using view_refl_types =
    composite_t<std::tuple<>, std::conditional_t<Rs::is_direct, void, Rs>...>;
template <facade... Fs>
using view_super_types = std::tuple<view_facade<Fs>...>;

struct weak_lock_dispatch {
  template <class P>
  PRO5D_STATIC_CALL(auto, const P& self) noexcept
    requires(requires {
      { self.lock() } noexcept;
    })
  {
    if constexpr (std::is_constructible_v<bool, decltype(self.lock())>) {
      return converter{
          [&self]<class F, class MP>(
              std::in_place_type_t<proxy<F, MP>>) noexcept -> proxy<F, MP> {
            auto strong = self.lock();
            return strong ? proxy<F, MP>{std::move(strong)} : proxy<F, MP>{};
          }};
    } else {
      return self.lock();
    }
  }
};
struct weak_lock_access {
  template <class Self, class Descriptor>
  struct accessor;
  template <class Self, class D, class R>
  struct accessor<Self, proxy_operation<D, R() const noexcept>> {
    PRO5D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(lock)
    R lock() const noexcept {
      return invoke<D, R() const noexcept>(static_cast<const Self&>(*this));
    }
  };
};

template <class WF, class MP>
using weak_lock_signature =
    proxy<typename WF::strong_type, MP>() const noexcept;
template <facade... Fs>
using weak_super_types = std::tuple<weak_facade<Fs>...>;

} // namespace detail

template <facade F>
struct view_facade
    : detail::facade_impl<
          detail::specialization_t<detail::view_super_types,
                                   typename F::super_types>,
          detail::specialization_t<detail::view_op_types,
                                   typename F::operation_types>,
          detail::specialization_t<detail::view_refl_types,
                                   typename F::reflection_types>,
          sizeof(void*), alignof(void*), lifetime_requirement::trivial,
          lifetime_requirement::trivial, lifetime_requirement::trivial> {};

template <facade F>
struct weak_facade
    : detail::facade_impl<
          detail::specialization_t<detail::weak_super_types,
                                   typename F::super_types>,
          std::tuple<detail::op_impl<
              true, detail::weak_lock_dispatch,
              proxy_dependent_signature<detail::weak_lock_signature>,
              detail::weak_lock_access>>,
          std::tuple<>, F::max_size, F::max_align, F::copyability,
          F::relocatability, F::destructibility> {
  using strong_type = F;
};

} // namespace pro::inline v5

#endif // MSFT_PROXY_V5_DETAIL_CORE_H_
