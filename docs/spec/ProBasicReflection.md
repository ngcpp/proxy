# Named requirements: *ProBasicReflection*

> Since: 3.1.0

A type `R` meets the *ProBasicReflection* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                  | Semantics                                                    |
| ---------------------------- | ------------------------------------------------------------ |
| `R::is_direct`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `bool`, specifying whether the reflection applies to a pointer type itself (`true`), or the element type of a pointer type (`false`). |
| `typename R::reflector_type` | A type that defines the data structure reflected from the type. Shall meet the [*ProBasicMetadata* requirements](ProBasicMetadata.md) *(since 5.0.0)*. |

*Since 5.0.0*: the *access type* of `R` is `typename R::access_type` if it is a valid type, or `void` otherwise. The access type provides an accessor for [`proxy`](proxy/README.md) for the reflection, together with the other conventions and reflections sharing it, when it meets the [*ProAccess* requirements](ProAccess.md). An access type that does not meet the requirements, such as `void`, provides no accessor.

## See Also

- [*ProAccess* requirements](ProAccess.md)
- [*ProBasicFacade* requirements](ProBasicFacade.md)
- [*ProBasicMetadata* requirements](ProBasicMetadata.md)
- [*ProReflection* requirements](ProReflection.md)
