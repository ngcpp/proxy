# Named requirements: *ProBasicMeta*

> Since: 5.0.0

A type `M` meets the *ProBasicMeta* requirements if the following expressions are well-formed and have the specified semantics (let `m` be a value of type `M`, `cm` be a value of type `const M`).

| Expressions | Semantics                                                    |
| ----------- | ------------------------------------------------------------ |
| `M()`       | Creates an object of type `M` holding unspecified metadata, shall not throw. |
| `M(cm)`     | Creates an object of type `M` holding the metadata of `cm`, shall not throw. |
| `m = cm`    | Replaces the metadata of `m` with the metadata of `cm`, shall not throw. |
| `m.~M()`    | Destroys the object `m`, shall not throw.                    |

## Notes

A "meta" is an object holding metadata deduced from a type at compile time, stored in or referenced by a [`proxy`](proxy/README.md). Because `proxy` creates, copies, assigns, and destroys metadata in contexts specified not to throw, none of these operations may throw.

## See Also

- [*ProBasicReflection* requirements](ProBasicReflection.md)
- [*ProMeta* requirements](ProMeta.md)
