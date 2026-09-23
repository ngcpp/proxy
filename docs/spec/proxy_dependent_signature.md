# Class template `proxy_dependent_signature`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
template <template <class, class> class O>
struct proxy_dependent_signature { proxy_dependent_signature() = delete; };
```

Class template `proxy_dependent_signature<O>` specifies a *proxy-dependent signature template* `O`. `O` is instantiated with a [facade](facade.md) type and a [metadata policy](ProMetadataPolicy.md) type, and shall produce a type meeting the [*ProOverload* requirements](ProOverload.md). It is useful when modeling an [overload](ProOverload.md) type of a facade type that recursively depends on the facade type itself, on the metadata policy of the [`proxy`](proxy/README.md) that exposes it, or on both.

*Since 5.0.0*: `proxy_dependent_signature` replaces `facade_aware_overload_t`, whose template argument was instantiated with a facade type only.

## Notes

`proxy_dependent_signature` can be used to define a convention in a base facade type, and is portable to the definition of another facade type via [`basic_facade_builder::add_facade`](basic_facade_builder/add_facade.md). It can also effectively avoid a facade type being implicitly instantiated when it is incomplete.

Instantiating `O` with the metadata policy in addition to the facade lets a convention return a `proxy` that keeps the metadata policy of the `proxy` it was obtained from, which is what [`skills::as_view`](skills_as_view.md) and [`skills::as_weak`](skills_as_weak.md) do.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

template <class F, class MP>
using BinarySignature =
    pro::proxy<F, MP>(const pro::proxy_indirect_accessor<F, MP>& rhs) const;

template <class T, pro::facade F, class MP>
pro::proxy<F, MP> operator+(const T& value,
                            const pro::proxy_indirect_accessor<F, MP>& rhs)
  requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F, MP>>)
{
  return pro::make_proxy<F, T, MP>(value + proxy_cast<const T&>(rhs));
}

struct Addable
    : pro::facade_builder              //
      ::add_skill<pro::skills::rtti>   //
      ::add_skill<pro::skills::format> //
      ::add_convention<pro::operator_dispatch<"+">,
                       pro::proxy_dependent_signature<BinarySignature>> //
      ::build {};

int main() {
  pro::proxy<Addable> p1 = pro::make_proxy<Addable>(1);
  pro::proxy<Addable> p2 = pro::make_proxy<Addable>(2);
  pro::proxy<Addable> p3 = *p1 + *p2;
  std::cout << std::format("{}\n", *p3); // Prints "3"
}
```

## See Also

- [*ProOverload* requirements](ProOverload.md)
- [alias template `skills::as_view`](skills_as_view.md)
