# Named requirements: *ProMetadata*

> Since: 5.0.0

A type `M` meets the *ProMetadata* requirements of a type `T` if `M` meets the [*ProBasicMetadata* requirements](ProBasicMetadata.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                | Semantics                                                    |
| -------------------------- | ------------------------------------------------------------ |
| `M(std::in_place_type<T>)` | Creates an object of type `M` holding implementation-defined metadata of type `T`, shall not throw. |

## See Also

- [*ProBasicMetadata* requirements](ProBasicMetadata.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
- [*ProReflection* requirements](ProReflection.md)
