# Named requirements: *ProxyOperation*

A type `O` meets the *ProxyOperation* requirements of a type `P` and a type `MP`, where `MP` meets the [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md), if `O` meets the [*ProxyBasicOperation* requirements](ProxyBasicOperation.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                   | Semantics                                                    |
| ----------------------------- | ------------------------------------------------------------ |
| `typename O::signature_type`  | A type whose [`substituted-signature`](ProxySignature.md)`<typename O::signature_type, F, MP>` is a function type `S` meeting the [*ProxySignature* requirements](ProxySignature.md), for the facade type `F` that declares `O`, where<br />- when `O::is_direct` is `true`, `typename O::dispatch_type` shall meet the [*ProxyDispatch* requirements](ProxyDispatch.md) of `P` and `S`, <br />- or otherwise, when `O::is_direct` is `false`, let `QP` be a qualified reference type of `P` with the *cv ref* qualifiers defined by `S` (`QP` is an lvalue reference type if `S` does not define a *ref* qualifier), `qp` be a value of `QP`, `*std::forward<QP>(qp)` shall be well-formed, and `typename O::dispatch_type` shall meet the [*ProxyDispatch* requirements](ProxyDispatch.md) of `decltype(*std::forward<QP>(qp))` and `S`. |

## See Also

- [*ProxyFacade* requirements](ProxyFacade.md)
