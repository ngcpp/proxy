# `basic_facade_builder::add_facade_with_substitution`

> Since: 4.1.0

```cpp
template <facade F>
using add_facade_with_substitution = basic_facade_builder</* see below */>;
```

The alias template `add_facade_with_substitution` of `basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` is equivalent to [`add_facade`](add_facade.md)`<F>`.

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
