# Named requirements: *ProxyFacade*

A type `F` meets the *ProxyFacade* requirements of a type `P` and a type `MP`, where `MP` meets the [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md), if `F` meets the [*ProxyBasicFacade* requirements](ProxyBasicFacade.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                                    | Semantics                                                    |
| ---------------------------------------------- | ------------------------------------------------------------ |
| `typename F::super_types`<br />*(since 5.0.0)* | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Fs`. Each type `G` in `Fs` shall meet the *ProxyFacade* requirements of `P` and `MP`. |
| `typename F::operation_types`                  | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Os`. Each type `O` in `Os` shall meet the [*ProxyOperation* requirements](ProxyOperation.md) of `P` and `MP`. |
| `typename F::reflection_types`                 | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Rs`. Each type `R` in `Rs` shall meet the [*ProxyReflection* requirements](ProxyReflection.md) of `P`. |
| `F::max_size`                                  | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that shall be greater than or equal to `sizeof(P)`. |
| `F::max_align`                                 | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that shall be greater than or equal to `alignof(P)`. |
| `F::copyability`                               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required copyability of `P`. |
| `F::relocatability`                            | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required relocatability of `P`. |
| `F::destructibility`                           | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required destructibility of `P`. |

*Since 5.0.0*: let `Os` be the operations of `F` and of every super of `F`, reachable via `typename F::super_types` transitively. Each type `O` in `Os` whose `typename O::signature_type` is a specialization of [`proxy_dependent_signature`](proxy_dependent_signature.md) shall also meet the [*ProxyOperation* requirements](ProxyOperation.md) of `P` with [`substituted-signature`](ProxySignature.md)`<typename O::signature_type, F, MP>` in place of `typename O::signature_type`. Among the types in `Os` sharing `is_direct` and `dispatch_type`, distinct `signature_type`s shall substitute to distinct signatures.

*Since 4.0.2*: `P` shall be a pointer-like type eligible for `proxy`. A type `P` is eligible if the following condition is satisfied:

```cpp
(std::is_pointer_v<T> || requires { typename T::element_type; } || requires(T val) { *val; }) && requires { typename std::pointer_traits<T>::element_type; }
```

In other words, `P` either supports dereferencing or provides an `element_type`, and `std::pointer_traits<P>` yields a valid `element_type`.

## Notes

Relocatability is defined as *move-construct an object and then destroy the original instance*. Specifically, the value of `F::relocatability` maps to the following requirements on `P`:

| Value                             | Requirement on `P`                                           |
| --------------------------------- | ------------------------------------------------------------ |
| `lifetime_requirement::none`      | None                                                         |
| `lifetime_requirement::supported` | `(std::is_move_constructible_v<P> && std::is_destructible_v<P>) || `[`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |
| `lifetime_requirement::nothrow`   | `(std::is_nothrow_move_constructible_v<P> && std::is_nothrow_destructible_v<P>) || `[`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |
| `lifetime_requirement::trivial`   | [`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |

## See Also

- [concept `proxiable`](proxiable.md)
- [*ProxyBasicFacade* requirements](ProxyBasicFacade.md)
