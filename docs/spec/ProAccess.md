# Named requirements: *ProAccess*

> Since: 5.0.0

A *descriptor* is a specialization of either [`proxy_operation`](proxy_operation.md) or [`proxy_reflection`](proxy_reflection.md). A type `A` meets the *ProAccess* requirements of a type `Self` and descriptor types `Ds...`, where `Self` is a specialization of either [`proxy`](proxy/README.md) or [`proxy_indirect_accessor`](proxy_indirect_accessor/README.md), if the following expressions are well-formed and have the specified semantics.

| Expressions                                  | Semantics                                                    |
| -------------------------------------------- | ------------------------------------------------------------ |
| `typename A::template accessor<Self, Ds...>` | A type whose member and non-member functions invoke the operations or acquire the reflections described by `Ds...` through `Self`. It shall be a *nothrow-default-constructible*, *trivially-copyable* type, and shall not be [final](https://en.cppreference.com/w/cpp/language/final). |

*Since 5.0.0*: *ProAccess* replaces *ProAccessible*, whose requirements a dispatch type `D` met with `typename D::template accessor<P, D, Os...>`, and a reflector type `R` with `typename R::template accessor<P, R>`. [`basic_facade_builder`](basic_facade_builder/README.md) gives the conventions and reflections it adds the default access types of their dispatch types and reflector types, which are formed from these member templates unless the library defines them (see [`add_convention`](basic_facade_builder/add_convention.md) and [`add_reflection`](basic_facade_builder/add_reflection.md)).

## Notes

The order of the descriptors supplied to an access type is unspecified. To provide the same functions across implementations, an access type should accept every permutation of those descriptors and provide equivalent member and non-member functions for each permutation. In particular, an accessor should identify an operation or reflection by its descriptor type rather than its position in `Ds...`.

The *access type* of a convention or a reflection is its member type `access_type`, or `void` if it has none (see [*ProBasicConvention*](ProBasicConvention.md) and [*ProBasicReflection*](ProBasicReflection.md)). For each access type `A`, `proxy` inherits a single accessor formed from the descriptors of all its direct conventions and reflections sharing `A`, and `proxy_indirect_accessor` inherits one formed from the descriptors of all its indirect conventions and reflections sharing `A`, provided that `A` meets the *ProAccess* requirements of `Self` and those descriptors (see [`proxy`](proxy/README.md) and [`proxy_indirect_accessor`](proxy_indirect_accessor/README.md)). Therefore, a member function of the accessor can be called on `Self`, and a friend function defined in the accessor can be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `Self` is an associated class of the arguments.

In the accessor, `*this` can be converted to a reference to `Self`, through which an operation described by `proxy_operation<D, O>` is performed via [`invoke<D, O>`](proxy/friend_invoke.md), and a reflection described by `proxy_reflection<M>` is acquired via [`reflect<M>`](proxy/friend_reflect.md). When `Ds...` contains more than one descriptor, the accessor usually inherits the accessors of the individual descriptors and, for member functions, makes them visible together with `using`-declarations, so that overload resolution is performed among all of them. [`operator_access`](operator_access/README.md), [`explicit_conversion_access`](explicit_conversion_access/README.md) and [`implicit_conversion_access`](implicit_conversion_access/README.md) follow this pattern.

These access types provide no accessor when any descriptor in `Ds...` describes an operation that they do not support. Since the direct, or the indirect, conventions sharing an access type share one accessor, a single unsupported convention removes the accessor for all of them. For example, `operator_access<"++">` supports only the overloads that take no argument (prefix `++`) or an `int` (postfix `++`), so adding a convention with the overload `void(double)` and the access type `operator_access<"++">` removes `operator++` from the other conventions with that access type.

On MSVC, only the first of several empty base classes is placed without taking up storage by default, so an accessor that inherits the accessors of more than one descriptor may increase the size of `Self`. Declaring such an accessor with [`__declspec(empty_bases)`](https://learn.microsoft.com/en-us/cpp/cpp/empty-bases) avoids this.

## See Also

- [class template `proxy`](proxy/README.md)
- [class template `proxy_indirect_accessor`](proxy_indirect_accessor/README.md)
- [class template `proxy_operation`](proxy_operation.md)
- [class template `proxy_reflection`](proxy_reflection.md)
