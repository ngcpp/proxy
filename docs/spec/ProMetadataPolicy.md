# Named requirements: *ProMetadataPolicy*

> Since: 5.0.0

A metadata policy determines how a [`proxy`](proxy/README.md) erases an invocation and how it keeps the metadata deduced from the contained type.

A nullable type is a type meeting the [*ProBasicMetadata* requirements](ProBasicMetadata.md) whose default-constructed value holds nothing, and for which `static_cast<bool>(v)` is a non-throwing expression that yields `false` if and only if `v` holds nothing.

A type `MP` meets the *ProMetadataPolicy* requirements if the following expressions are well-formed and have the specified semantics, where

- `Ctx` is an implementation-defined *erased context* type that identifies the contained value of a `proxy`, and for which an implementation-defined function template `invoke` is found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl),
- `O` is a type meeting the [*ProOverload* requirements](ProOverload.md), `R` is the return type of `O` and `Args...` are the argument types of `O`,
- `P` is a pointer type eligible for `proxy` (see [*ProFacade* requirements](ProFacade.md)),
- `M` is a nullable type meeting the [*ProBasicMetadata* requirements](ProBasicMetadata.md) and the [*ProMetadata* requirements](ProMetadata.md) of `P`.

| Expressions                             | Semantics                                                    |
| --------------------------------------- | ------------------------------------------------------------ |
| `typename MP::template invoker<Ctx, O>` | A nullable type `I` meeting the [*ProBasicMetadata* requirements](ProBasicMetadata.md). `I` shall not be [final](https://en.cppreference.com/w/cpp/language/final). |
| `typename MP::template storage<M>`      | A nullable type `S` meeting the [*ProBasicMetadata* requirements](ProBasicMetadata.md). |

Let `ci` be a value of type `const I`, `ctx` be a value of type `Ctx`, `args...` be values of type `Args...`, `s` be a value of type `S`, and `cs` be a value of type `const S`. The following expressions shall be well-formed and have the specified semantics.

| Expressions                            | Semantics                                                    |
| -------------------------------------- | ------------------------------------------------------------ |
| `I(std::in_place_type<P>)`             | Creates an object of type `I` that holds an invoker of `P`, shall not throw. |
| `ci(ctx, std::forward<Args>(args)...)` | Has the same effect as `invoke<P>(ctx, std::forward<Args>(args)...)`, where `P` is the type `ci` was created with, and the return type is `R`. Shall not throw when `O` is a `noexcept` overload. The behavior is undefined when `ci` holds no invoker. |
| `S(std::in_place_type<P>)`             | Creates an object of type `S` that holds the metadata `M(std::in_place_type<P>)`, shall not throw. |
| `*cs`                                  | A `const M&` referring to the metadata held by `cs`. The behavior is undefined when `cs` holds no metadata. |
| `s = cs2`                              | Where `cs2` is a value of type `const MP::template storage<M2>`, for some type `M2` whose `const M2&` is [nothrow-convertible](https://en.cppreference.com/w/cpp/types/is_convertible) to `const M&`. Replaces the metadata of `s` with the metadata of `cs2` converted to `M`, shall not throw. The behavior is undefined when `cs2` holds no metadata. |

## Notes

`I` is required not to be final because `proxy` composes the invokers of all the conventions of a facade into a single metadata object by inheritance.

The last expression is what makes a conversion to a super well-formed. The metadata of a super is reachable from the metadata of the deriving facade, so assigning the latter to the former transfers the invokers the super needs.

The metadata type of a [`proxy`](proxy/README.md)`<F, MP>` depends on both `F` and `MP`, so a `proxy` converts only to a `proxy` with the same metadata policy. The metadata policy is therefore chosen once, where the `proxy` type is named, and every conversion from that point on stays within it.

## See Also

- [class `compact_metadata`<br />class `inline_metadata`](compact_metadata.md)
- [*ProBasicMetadata* requirements](ProBasicMetadata.md)
- [*ProMetadata* requirements](ProMetadata.md)
