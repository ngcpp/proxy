# Class template `proxy_indirect_accessor`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 3.2.0

```cpp
template <facade F, class MP = compact_metadata>
class proxy_indirect_accessor;
```

Class template `proxy_indirect_accessor` provides accessors for the indirect operations and reflections of `proxy`. Let `Os` be the operation types of `F` and of every super of `F`, reachable via `typename F::super_types` transitively, and `Rs` be the reflection types of `F` and of every such super. The *descriptor* of a type `O` in `Os` is [`proxy_operation`](../proxy_operation.md)`<typename O::dispatch_type, `[`substituted-signature`](../ProxySignature.md)`<typename O::signature_type, F, MP>>`, and the *descriptor* of a type `R` in `Rs` is [`proxy_reflection`](../proxy_reflection.md)`<typename R::metadata_type>`.

The *access type* of a type `O` in `Os` or `R` in `Rs` is `typename O::access_type` or `typename R::access_type` if it is a valid type, or `void` otherwise (see [*ProxyBasicOperation*](../ProxyBasicOperation.md) and [*ProxyBasicReflection*](../ProxyBasicReflection.md)). For each distinct access type `A` of the types `O` in `Os` where `O::is_direct` is `false` and of the types `R` in `Rs` where `R::is_direct` is `false`, let `Ds...` be the descriptors of those operations and reflections whose access type is `A`, with duplicates removed and in an unspecified order. If `A` meets the [*ProxyAccess* requirements](../ProxyAccess.md) of `proxy_indirect_accessor<F, MP>` and `Ds...`, `typename A::template accessor<proxy_indirect_accessor<F, MP>, Ds...>` is inherited by `proxy_indirect_accessor<F, MP>`.


*Since 5.0.0*: `Os` and `Rs` include the operations and reflections of the supers of `F`, and each accessor is provided by an access type and formed from the descriptors of every operation and reflection in `Os` and `Rs` sharing that access type. Previously, each accessor was provided by the dispatch type of a single operation or by the metadata type of a single reflection. `proxy_indirect_accessor` also takes a metadata policy.

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## Non-Member Functions

| Name                                                 | Description                                                  |
| ---------------------------------------------------- | ------------------------------------------------------------ |
| [`invoke`](friend_invoke.md)                         | invokes a `proxy` with a specified operation                |
| [`reflect`](friend_reflect.md)                       | acquires reflection information of a contained type          |

## Notes

Two indirect operations sharing an access type contribute the same descriptor only if they also share `dispatch_type` and `signature_type`, because the [*ProxyFacade* requirements](../ProxyFacade.md) require distinct signature types of the operations sharing `is_direct` and `dispatch_type` to substitute to distinct signatures. Such duplicates arise when the same operation is declared more than once, for example by several supers of `F`.

## See also

- [class template `proxy`](../proxy/README.md)
