# Named requirements: *ProxyReflection*

> Since: 3.1.0

A type `R` meets the *ProxyReflection* requirements of a type `P` if `R` meets the [*ProxyBasicReflection* requirements](ProxyBasicReflection.md), and the following expressions are well-formed and have the specified semantics (let `T` be `P` when `R::is_direct` is `true`, or otherwise `typename std::pointer_traits<P>::element_type`).

| Expressions                                         | Semantics                                                    |
| --------------------------------------------------- | ------------------------------------------------------------ |
| `typename R::metadata_type(std::in_place_type<T>)`  | Constructs a value of type `typename R::metadata_type`, reflecting implementation-defined metadata of type `T`. `typename R::metadata_type` shall meet the [*ProxyMetadata* requirements](ProxyMetadata.md) of `T` *(since 5.0.0)*. |

## See Also

- [*ProxyFacade* requirements](ProxyFacade.md)
- [*ProxyMetadata* requirements](ProxyMetadata.md)
