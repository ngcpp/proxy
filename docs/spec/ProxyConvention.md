# Named requirements: *ProxyConvention*

A type `C` meets the *ProxyConvention* requirements of a type `P` and a type `MP`, where `MP` meets the [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md), if `C` meets the [*ProxyBasicConvention* requirements](ProxyBasicConvention.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                  | Semantics                                                    |
| ---------------------------- | ------------------------------------------------------------ |
| `typename C::overload_type`  | A type whose [`substituted-overload`](ProxyOverload.md)`<typename C::overload_type, F, MP>` is a function type `O` meeting the [*ProxyOverload* requirements](ProxyOverload.md), for the facade type `F` that declares `C`, where<br />- when `C::is_direct` is `true`, `typename C::dispatch_type` shall meet the [*ProxyDispatch* requirements](ProxyDispatch.md) of `P` and `O`, <br />- or otherwise, when `C::is_direct` is `false`, let `QP` be a qualified reference type of `P` with the *cv ref* qualifiers defined by `O` (`QP` is an lvalue reference type if `O` does not define a *ref* qualifier), `qp` be a value of `QP`, `*std::forward<QP>(qp)` shall be well-formed, and `typename C::dispatch_type` shall meet the [*ProxyDispatch* requirements](ProxyDispatch.md) of `decltype(*std::forward<QP>(qp))` and `O`. |

## See Also

- [*ProxyFacade* requirements](ProxyFacade.md)
