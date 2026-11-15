# Named requirements: *ProxyBasicOperation*

A type `O` meets the *ProxyBasicOperation* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                                       | Semantics                                                    |
| ------------------------------------------------- | ------------------------------------------------------------ |
| `O::is_direct`                                    | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `bool`, specifying whether the operation applies to a pointer type itself (`true`), or the element type of a pointer type (`false`). |
| `typename O::dispatch_type`                       | A type that defines how the calls are forwarded to the concrete types. Shall be *nothrow-default-constructible* and *nothrow-destructible*. |
| `typename O::signature_type`<br />*(since 5.0.0)* | A type that meets the [*ProxySignature* requirements](ProxySignature.md). |

*Since 5.0.0*: the *access type* of `O` is `typename O::access_type` if it is a valid type, or `void` otherwise. The access type provides an accessor for [`proxy`](proxy/README.md) for the operation, together with the other operations and reflections sharing it, when it meets the [*ProxyAccess* requirements](ProxyAccess.md). An access type that does not meet the requirements, such as `void`, provides no accessor.

## See Also

- [*ProxyAccess* requirements](ProxyAccess.md)
- [*ProxyBasicFacade* requirements](ProxyBasicFacade.md)
- [*ProxyOperation* requirements](ProxyOperation.md)
