# Named requirements: *ProOverload*

A type `O` meets the *ProOverload* requirements if `substituted-overload<O, F, MP>` matches one of the following definitions, where `F` is any type meeting the [*ProBasicFacade* requirements](ProBasicFacade.md), `MP` is any type meeting the [*ProMetadataPolicy* requirements](ProMetadataPolicy.md), `R` is the *return type*, `Args...` are the *argument types*.

*Since 5.0.0*: the exposition-only type `substituted-overload<O, F, MP>` is `OT<F, MP>` if `O` is a specialization of [`proxy_dependent_signature<OT>`](proxy_dependent_signature.md), or `O` otherwise. Previously the exposition-only type was `substituted-overload<O, F>`, and `OT` was instantiated with `F` only.

| Definitions of `substituted-overload<O, F, MP>` |
| ----------------------------------------------- |
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

- [*ProConvention* requirements](ProConvention.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
- [class template `std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function)
