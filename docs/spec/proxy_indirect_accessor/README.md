# Class template `proxy_indirect_accessor`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
template <facade F>
class proxy_indirect_accessor;
```

Class template `proxy_indirect_accessor` provides indirection accessibility for `proxy`. As per `facade<F>`, `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Cs`, and `typename F::reflection_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Rs`.

- For each distinct dispatch type `D` among the types `C` in `Cs` where `C::is_direct` is `false`, let `Os...` be the overload types of those conventions with duplicates removed, and `substituted-overload-types...` be [`substituted-overload<Os, F>...`](../ProOverload.md). If `D` meets the [*ProAccessible* requirements](../ProAccessible.md) of `proxy_indirect_accessor<F>, D, substituted-overload-types...`, `typename D::template accessor<proxy_indirect_accessor<F>, D, substituted-overload-types...>` is inherited by `proxy_indirect_accessor<F>`.
- For each type `R` in `Rs`, if `R::is_direct` is `false` and `typename R::reflector_type` meets the [*ProAccessible* requirements](../ProAccessible.md) of `proxy_indirect_accessor<F>, typename R::reflector_type`, `typename R::reflector_type::template accessor<proxy_indirect_accessor<F>, typename R::reflector_type` is inherited by `proxy_indirect_accessor<F>`.

*Since 5.0.0*: the accessor of a dispatch type is formed from the overload types of every convention in `Cs` sharing that dispatch type, rather than from a single convention.

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## Non-Member Functions

| Name                                                 | Description                                                  |
| ---------------------------------------------------- | ------------------------------------------------------------ |
| [`invoke`](friend_invoke.md)                         | invokes a `proxy` with a specified convention                |
| [`reflect`](friend_reflect.md)                       | acquires reflection information of a contained type          |

## See also

- [class template `proxy`](../proxy/README.md)
