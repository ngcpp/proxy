# Named requirements: *ProBasicReflection*

> Since: 3.1.0

A type `R` meets the *ProBasicReflection* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                  | Semantics                                                    |
| ---------------------------- | ------------------------------------------------------------ |
| `R::is_direct`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `bool`, specifying whether the reflection applies to a pointer type itself (`true`), or the element type of a pointer type (`false`). |
| `typename R::reflector_type` | A type that defines the data structure reflected from the type. Shall meet the [*ProBasicMeta* requirements](ProBasicMeta.md) *(since 5.0.0)*. |

## See Also

- [*ProBasicFacade* requirements](ProBasicFacade.md)
- [*ProBasicMeta* requirements](ProBasicMeta.md)
- [*ProReflection* requirements](ProReflection.md)
