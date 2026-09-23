module;

#include <proxy/v5/proxy.h>

export module proxy.v5;

export namespace pro::inline v5 {

#if __STDC_HOSTED__
using v5::allocate_proxy;
using v5::allocate_proxy_shared;
using v5::make_proxy;
using v5::make_proxy_shared;
#endif // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
using v5::bad_proxy_cast;
#endif // __cpp_rtti >= 199711L

using v5::basic_facade_builder;
using v5::compact_metadata;
using v5::constraint_level;
using v5::conversion_dispatch;
using v5::explicit_conversion_dispatch;
using v5::facade;
using v5::facade_builder;
using v5::implicit_conversion_dispatch;
using v5::inline_metadata;
using v5::inplace_proxiable_target;
using v5::is_bitwise_trivially_relocatable;
using v5::is_bitwise_trivially_relocatable_v;
using v5::make_proxy_inplace;
using v5::make_proxy_view;
using v5::not_implemented;
using v5::observer_facade;
using v5::operator_dispatch;
using v5::proxiable;
using v5::proxiable_target;
using v5::proxy;
using v5::proxy_dependent_signature;
using v5::proxy_indirect_accessor;
using v5::proxy_invoke;
using v5::proxy_reflect;
using v5::proxy_view;
using v5::weak_dispatch;
using v5::weak_facade;
using v5::weak_proxy;

namespace skills {

#ifdef PRO5D_HAS_FORMAT
using skills::format;
using skills::wformat;
#endif // PRO5D_HAS_FORMAT

#if __cpp_rtti >= 199711L
using skills::direct_rtti;
using skills::indirect_rtti;
using skills::rtti;
#endif // __cpp_rtti >= 199711L

using skills::as_view;
using skills::as_weak;
using skills::slim;

} // namespace skills

} // namespace pro::inline v5

#ifdef PRO5D_HAS_FORMAT
export namespace std {

using std::formatter;

} // namespace std
#endif // PRO5D_HAS_FORMAT
