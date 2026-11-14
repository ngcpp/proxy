# `basic_facade_builder::add_convention`<br />`basic_facade_builder::add_direct_convention`<br />`basic_facade_builder::add_indirect_convention`

```cpp
template <class D, class... Os> requires(/* see below */)
using add_convention = add_indirect_convention<D, Os...>;

template <class D, class... Os> requires(/* see below */)
using add_indirect_convention = basic_facade_builder</* see below */>;

template <class D, class... Os> requires(/* see below */)
using add_direct_convention = basic_facade_builder</* see below */>;
```

The alias templates `add_convention`, `add_indirect_convention`, and `add_direct_convention` of `basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` add convention types to the template parameters. The expression inside `requires` is equivalent to `sizeof...(Os) > 0u` and each type in `Os` meets the [*ProOverload* requirements](../ProOverload.md).

- `add_convention` is equivalent to `add_indirect_convention`.
- `add_indirect_convention` appends an implementation-defined convention type `IC` to `Cs` for each type `O` in `Os`, where:
  - `IC::is_direct` is `false`.
  - `typename IC::dispatch_type` is `D`.
  - `typename IC::overload_type` is `O`.
- `add_direct_convention` appends an implementation-defined convention type `IC` to `Cs` for each type `O` in `Os`, where:
  - `IC::is_direct` is `true`.
  - `typename IC::dispatch_type` is `D`.
  - `typename IC::overload_type` is `O`.

For either alias, `typename IC::access_type` is [`operator_access<Sign, Rhs>`](../operator_access/README.md) if `D` is [`operator_dispatch<Sign, Rhs>`](../operator_dispatch/README.md), [`explicit_conversion_access`](../explicit_conversion_access/README.md) if `D` is [`explicit_conversion_dispatch`](../explicit_conversion_dispatch/README.md), [`implicit_conversion_access`](../implicit_conversion_access/README.md) if `D` is [`implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md), [`member_access<Name>`](../member_access.md) if `D` is [`member_dispatch<Name>`](../member_dispatch.md), and [`free_access<Name>`](../free_access.md) if `D` is [`free_dispatch<Name>`](../free_dispatch.md). It is implementation-defined for the dispatch types that the library uses to implement its skills. If `D` is [`weak_dispatch<D2>`](../weak_dispatch/README.md) and the builder selects a library-defined access type for `D2` by these rules, `IC::access_type` is the same type. For any other `D`, it is an implementation-defined type `A` where `typename A::template accessor<Self, `[`proxy_operation`](../proxy_operation.md)`<D, Os>...>` is `typename D::template accessor<Self, D, Os...>`.

*Since 5.0.0*: each type in `Os` produces its own convention type, rather than one convention type carrying a tuple-like `overload_types`. The convention type carries an access type (see [*ProBasicConvention* requirements](../ProBasicConvention.md)). Previously, `proxy` took the accessibility of the convention from the member template `accessor` of `D`.

## Notes

Adding duplicated combinations of some dispatch type and overload type is well-defined (either directly via `add_convention`, `add_indirect_convention`, `add_direct_convention`, or indirectly via [`add_facade`](add_facade.md)). While such duplicates change the type produced by [`build`](build.md), they do not have side-effects on a [`proxy`](../proxy/README.md) of that facade at either compile-time or runtime.

## Example

```cpp
#include <iostream>
#include <memory>
#include <string>

#include <proxy/proxy.h>

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct BasicStringable : pro::facade_builder                                 //
                         ::add_convention<FreeToString, std::string() const> //
                         ::build {};

struct Stringable : pro::facade_builder                               //
                    ::add_facade<BasicStringable>                     //
                    ::support_copy<pro::constraint_level::nontrivial> //
                    ::add_direct_convention<pro::conversion_dispatch,
                                            pro::proxy<BasicStringable>() &&> //
                    ::build {};

int main() {
  pro::proxy<Stringable> p1 = std::make_shared<int>(123);
  pro::proxy<Stringable> p2 = p1;
  pro::proxy<BasicStringable> p3 =
      static_cast<pro::proxy<BasicStringable>>(std::move(p2));
  pro::proxy<BasicStringable> p4 = std::move(p3);
  // pro::proxy<BasicStringable> p5 = p4; // Won't compile
  std::cout << ToString(*p4) << "\n";                    // Prints "123"
  std::cout << std::boolalpha << p3.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [macro `PRO_DEF_MEM_DISPATCH`](../PRO_DEF_MEM_DISPATCH.md)
- [macro `PRO_DEF_FREE_DISPATCH`](../PRO_DEF_FREE_DISPATCH.md)
- [class template `operator_dispatch`](../operator_dispatch/README.md)
- [class `conversion_dispatch`](../explicit_conversion_dispatch/README.md)
