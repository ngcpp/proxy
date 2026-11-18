# `basic_facade_builder::support_destruction`

```cpp
template <lifetime_requirement LR>
    requires(/* see below */)
using support_destruction = basic_facade_builder</* see below */>;
```

The alias template `support_destruction` of `basic_facade_builder<Fs, Os, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` adds destruction support to the template parameters. After the operation, `Destructibility` becomes `std::max(Destructibility, LR)`. The expression inside `requires` is equivalent to `LR` is a defined enumerator of `lifetime_requirement`.

## Notes

If no destructibility support is applied before specifying [`build`](build.md), the default value of `build::destructibility` is `pro::lifetime_requirement::nothrow`.

## Example

```cpp
#include <type_traits>

#include <proxy/proxy.h>

struct Movable : pro::facade_builder::build {};

struct NonriviallyDestructible
    : pro::facade_builder                                         //
      ::support_relocation<pro::lifetime_requirement::supported>  //
      ::support_destruction<pro::lifetime_requirement::supported> //
      ::build {};

int main() {
  static_assert(std::is_nothrow_destructible_v<pro::proxy<Movable>>);
  static_assert(
      !std::is_nothrow_destructible_v<pro::proxy<NonriviallyDestructible>>);
}
```

## See Also

- [`support_relocation`](support_relocation.md)
