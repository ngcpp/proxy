# Concept `proxiable`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`

```cpp
template <class P, class F, class MP = compact_metadata>
concept proxiable = /* see-below */;
```

The concept `proxiable<P, F, MP>` specifies that [`proxy<F, MP>`](proxy/README.md) can potentially contain a value of type `P`. If `P` is an incomplete type, the behavior of evaluating `proxiable<P, F, MP>` is undefined. `proxiable<P, F, MP>` is `true` when `F` meets the [*ProFacade* requirements](ProFacade.md) of `P` and `MP`; otherwise, it is `false`.

*Since 5.0.0*: `proxiable` takes a [metadata policy](ProMetadataPolicy.md), which participates in the check when a convention of `F` is declared with a [`proxy_dependent_signature`](proxy_dependent_signature.md). `MP` is not itself checked by `proxiable`, and a type that does not meet the *ProMetadataPolicy* requirements is diagnosed where `proxy<F, MP>` is instantiated.

## Example

```cpp
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder                           //
                    ::add_convention<FreeToString, std::string()> //
                    ::build {};

int main() {
  static_assert(pro::proxiable<int*, Stringable>);
  static_assert(pro::proxiable<std::shared_ptr<double>, Stringable>);
  static_assert(!pro::proxiable<std::vector<int>*, Stringable>);
}
```

## See Also

- [class template `proxy`](proxy/README.md)
- [function template `make_proxy`](make_proxy.md)
- [concept `inplace_proxiable_target`](inplace_proxiable_target.md)
