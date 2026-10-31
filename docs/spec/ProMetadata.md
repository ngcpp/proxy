# Named requirements: *ProMetadata*

> Since: 5.0.0

A type `M` meets the *ProMetadata* requirements of a type `T` if `M` meets the [*ProBasicMetadata* requirements](ProBasicMetadata.md), and the following expression is well-formed, non-throwing, and has the specified semantics.

| Expression                 | Semantics                                                    |
| -------------------------- | ------------------------------------------------------------ |
| `M(std::in_place_type<T>)` | Creates an object of type `M` holding metadata deduced from `T`. |

## See Also

- [*ProBasicMetadata* requirements](ProBasicMetadata.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
- [*ProReflection* requirements](ProReflection.md)
