# Named requirements: *ProxyBasicFacade*

A type `F` meets the *ProxyBasicFacade* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                                    | Semantics                                                    |
| ---------------------------------------------- | ------------------------------------------------------------ |
| `typename F::super_types`<br />*(since 5.0.0)* | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Fs`. Each type `G` in `Fs` shall meet the *ProxyBasicFacade* requirements, and shall be no more conservative than `F`, that is: `F::max_size <= G::max_size`, `F::max_align <= G::max_align`, `F::copyability >= G::copyability`, `F::relocatability >= G::relocatability`, and `F::destructibility >= G::destructibility`. |
| `typename F::operation_types`                  | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Os`. Each type `O` in `Os` shall meet the [*ProxyBasicOperation* requirements](ProxyBasicOperation.md). |
| `typename F::reflection_types`                 | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of types `Rs`. Each type `R` in `Rs` shall meet the [*ProxyBasicReflection* requirements](ProxyBasicReflection.md). |
| `F::max_size`                                  | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that defines the maximum size of a pointer type. Shall be greater than `0` and a multiple of `F::max_align`. |
| `F::max_align`                                 | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that defines the maximum alignment of a pointer type. Shall be a power of `2`. |
| `F::copyability`                               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required copyability of a pointer type. |
| `F::relocatability`                            | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required relocatability of a pointer type. |
| `F::destructibility`                           | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`lifetime_requirement`](lifetime_requirement.md) that defines the required destructibility of a pointer type. |

Each of `F::copyability`, `F::relocatability`, and `F::destructibility` shall be exactly one of the four values of `lifetime_requirement` (`none`, `supported`, `nothrow`, `trivial`).

## See Also

- [concept `facade`](facade.md)
