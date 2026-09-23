# Named requirements: *ProBasicMetadata*

> Since: 5.0.0

A type `M` meets the *ProBasicMetadata* requirements if `M` is a class type, and the following expressions are well-formed and have the specified semantics (let `m` be a value of type `M`, `cm` be a value of type `const M`).

| Expressions | Semantics                                                    |
| ----------- | ------------------------------------------------------------ |
| `M()`       | Creates an object of type `M` holding unspecified metadata, shall not throw. |
| `M(cm)`     | Creates an object of type `M` holding the metadata of `cm`, shall not throw. |
| `m = cm`    | Replaces the metadata of `m` with the metadata of `cm`, shall not throw. |
| `m.~M()`    | Destroys the object `m`, shall not throw.                    |

## Notes

"Metadata" is an object holding information deduced from a type at compile time, stored in or referenced by a [`proxy`](proxy/README.md). Because `proxy` creates, copies, assigns, and destroys metadata in contexts specified not to throw, none of these operations may throw.

## See Also

- [*ProBasicReflection* requirements](ProBasicReflection.md)
- [*ProMetadata* requirements](ProMetadata.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
