# Class template `proxy_indirect_accessor`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 3.2.0

```cpp
template <facade F, class MP = compact_metadata>
class proxy_indirect_accessor;
```

Class template `proxy_indirect_accessor` provides accessors for the indirect conventions and reflections of `proxy`. Let `Cs` be the convention types of `F` and of every super of `F`, reachable via `typename F::super_types` transitively, and `Rs` be the reflection types of `F` and of every such super. The *descriptor* of a type `C` in `Cs` is [`proxy_operation`](../proxy_operation.md)`<typename C::dispatch_type, `[`substituted-overload`](../ProOverload.md)`<typename C::overload_type, F, MP>>`, and the *descriptor* of a type `R` in `Rs` is [`proxy_reflection`](../proxy_reflection.md)`<typename R::reflector_type>`.

The *access type* of a type `C` in `Cs` or `R` in `Rs` is `typename C::access_type` or `typename R::access_type` if it is a valid type, or `void` otherwise (see [*ProBasicConvention*](../ProBasicConvention.md) and [*ProBasicReflection*](../ProBasicReflection.md)). For each distinct access type `A` of the types `C` in `Cs` where `C::is_direct` is `false` and of the types `R` in `Rs` where `R::is_direct` is `false`, let `Ds...` be the descriptors of those conventions and reflections whose access type is `A`, with duplicates removed and in an unspecified order. If `A` meets the [*ProAccess* requirements](../ProAccess.md) of `proxy_indirect_accessor<F, MP>` and `Ds...`, `typename A::template accessor<proxy_indirect_accessor<F, MP>, Ds...>` is inherited by `proxy_indirect_accessor<F, MP>`.


*Since 5.0.0*: `Cs` and `Rs` include the conventions and reflections of the supers of `F`, and each accessor is provided by an access type and formed from the descriptors of every convention and reflection in `Cs` and `Rs` sharing that access type. Previously, each accessor was provided by the dispatch type of a single convention or by the reflector type of a single reflection. `proxy_indirect_accessor` also takes a metadata policy.

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## Non-Member Functions

| Name                                                 | Description                                                  |
| ---------------------------------------------------- | ------------------------------------------------------------ |
| [`invoke`](friend_invoke.md)                         | invokes a `proxy` with a specified convention                |
| [`reflect`](friend_reflect.md)                       | acquires reflection information of a contained type          |

## Notes

Two indirect conventions sharing an access type contribute the same descriptor only if they also share `dispatch_type` and `overload_type`, because the [*ProFacade* requirements](../ProFacade.md) require distinct overload types of the conventions sharing `is_direct` and `dispatch_type` to substitute to distinct overloads. Such duplicates arise when the same convention is declared more than once, for example by several supers of `F`.

## See also

- [class template `proxy`](../proxy/README.md)
