# Named requirements: *ProBasicConvention*

A type `C` meets the *ProBasicConvention* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                                      | Semantics                                                    |
| ------------------------------------------------ | ------------------------------------------------------------ |
| `C::is_direct`                                   | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `bool`, specifying whether the convention applies to a pointer type itself (`true`), or the element type of a pointer type (`false`). |
| `typename C::dispatch_type`                      | A type that defines how the calls are forwarded to the concrete types. Shall be *nothrow-default-constructible* and *nothrow-destructible*. |
| `typename C::overload_type`<br />*(since 5.0.0)* | A type that meets the [*ProOverload* requirements](ProOverload.md). |

## See Also

- [*ProBasicFacade* requirements](ProBasicFacade.md)
- [*ProConvention* requirements](ProConvention.md)
