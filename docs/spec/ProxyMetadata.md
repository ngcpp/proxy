# Named requirements: *ProxyMetadata*

> Since: 5.0.0

A type `M` meets the *ProxyMetadata* requirements of a type `T` if `M` meets the [*ProxyBasicMetadata* requirements](ProxyBasicMetadata.md), and the following expression is well-formed, non-throwing, and has the specified semantics.

| Expression                 | Semantics                                                    |
| -------------------------- | ------------------------------------------------------------ |
| `M(std::in_place_type<T>)` | Creates an object of type `M` holding metadata deduced from `T`. |

## See Also

- [*ProxyBasicMetadata* requirements](ProxyBasicMetadata.md)
- [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md)
- [*ProxyReflection* requirements](ProxyReflection.md)
