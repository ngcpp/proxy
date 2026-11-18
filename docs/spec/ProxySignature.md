# Named requirements: *ProxySignature*

A type `S` meets the *ProxySignature* requirements if `substituted-signature<S, F, MP>` matches one of the following definitions, where `F` is any type meeting the [*ProxyBasicFacade* requirements](ProxyBasicFacade.md), `MP` is any type meeting the [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md), `R` is the *return type*, `Args...` are the *argument types*.

*Since 5.0.0*: the exposition-only type `substituted-signature<S, F, MP>` is `ST<F, MP>` if `S` is a specialization of [`proxy_dependent_signature<ST>`](proxy_dependent_signature.md), or `S` otherwise. Previously the exposition-only type was `substituted-overload<S, F>`, and `ST` was instantiated with `F` only.

| Definitions of `substituted-signature<S, F, MP>` |
| ------------------------------------------------ |
| `R(Args...)`                                |
| `R(Args...) noexcept`                       |
| `R(Args...) &`                              |
| `R(Args...) & noexcept`                     |
| `R(Args...) &&`                             |
| `R(Args...) && noexcept`                    |
| `R(Args...) const`                          |
| `R(Args...) const noexcept`                 |
| `R(Args...) const&`                         |
| `R(Args...) const& noexcept`                |
| `R(Args...) const&&`                        |
| `R(Args...) const&& noexcept`               |

## See Also

- [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md)
- [*ProxyOperation* requirements](ProxyOperation.md)
- [class template `std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function)
