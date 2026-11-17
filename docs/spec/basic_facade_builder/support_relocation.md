# `basic_facade_builder::support_relocation`

```cpp
template <lifetime_requirement LR>
    requires(/* see below */)
using support_relocation = basic_facade_builder</* see below */>;
```

The alias template `support_relocation` of `basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` adds relocatability support to the template parameters. After the operation, `Relocatability` becomes `std::max(Relocatability, LR)`. The expression inside `requires` is equivalent to `LR` is a defined enumerator of `lifetime_requirement`.

## Notes

If no relocatability support is applied before specifying [`build`](build.md), the default value of `build::relocatability` is `pro::lifetime_requirement::trivial`. Please refer to [the *ProxyFacade* requirements](../ProxyFacade.md) for more details.

## Example

```cpp
#include <memory>

#include <proxy/proxy.h>

struct Movable : pro::facade_builder::build {};

struct Trivial : pro::facade_builder                                       //
                 ::support_copy<pro::lifetime_requirement::trivial>        //
                 ::support_relocation<pro::lifetime_requirement::trivial>  //
                 ::support_destruction<pro::lifetime_requirement::trivial> //
                 ::build {};

int main() {
  pro::proxy<Movable> p1 = std::make_unique<int>(123);
  // pro::proxy<Trivial> p2 = std::make_unique<int>(456);  // Won't compile
  double v = 3.14;
  pro::proxy<Trivial> p3 = &v; // Compiles because double* is trivial
}
```

## See Also

- [`support_copy`](support_copy.md)
