# `basic_facade_builder::add_facade_with_substitution`

> Since: 4.1.0

```cpp
template <facade F>
using add_facade_with_substitution = basic_facade_builder</* see below */>;
```

The alias template `add_facade_with_substitution` of `basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` is equivalent to [`add_facade`](add_facade.md)`<F>`, except that it always merges a direct convention of [`substitution_dispatch`](../substitution_dispatch/README.md) into `Cs`. This convention enables substitution from a `proxy` of the built [facade](../facade.md) to a `proxy<F>`.

## Notes

`add_facade_with_substitution` was introduced in `4.1.0` as a replacement for the deprecated `add_facade<F, true>` syntax.

The substitution convention is helpful when an API requires backward compatibility, at the cost of potentially a slightly larger binary size. When substitution is not required, use [`add_facade`](add_facade.md) to guarantee minimal binary size in code generation.

## Example

```cpp
#include <iostream>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemClear, clear);

struct Container : pro::facade_builder                            //
                   ::add_convention<MemSize, std::size_t() const> //
                   ::build {};

// A proxy<ClearableContainer> can be substituted with a proxy<Container>.
struct ClearableContainer : pro::facade_builder                       //
                            ::add_facade_with_substitution<Container> //
                            ::add_convention<MemClear, void()>        //
                            ::build {};

int main() {
  pro::proxy<ClearableContainer> p1 =
      pro::make_proxy<ClearableContainer, std::vector<int>>(10);
  std::cout << p1->size() << "\n"; // Prints "10"

  // Substitution from an rvalue: ClearableContainer -> Container
  pro::proxy<Container> p2 = std::move(p1);
  std::cout << p2->size() << "\n"; // Prints "10"
}
```

## See Also

- [`add_facade`](add_facade.md)
- [`build`](build.md)
