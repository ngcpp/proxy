# Class `implicit_conversion_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
struct implicit_conversion_access;
```

Class `implicit_conversion_access` is an access type that defines implicit conversion functions for [`proxy`](../proxy/README.md). It meets the [*ProAccess* requirements](../ProAccess.md) of applicable types.

`implicit_conversion_access` is the access type of the conventions of [`implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md) added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md). A convention can also name it as its access type with any other [dispatch](../ProDispatch.md) type that converts a `proxy` to the return types of its overloads. The direct, or the indirect, conventions whose access type is `implicit_conversion_access` share an accessor (see [*ProAccess* requirements](../ProAccess.md)), so the conversions of all of them are available together.

## Member Types

| Name                      | Description                       |
| ------------------------- | --------------------------------- |
| [`accessor`](accessor.md) | defines functions for `proxy` |

## Example

```cpp
#include <iostream>
#include <string>
#include <tuple>

#include <proxy/proxy.h>

struct ToStringDispatch {
  template <class T>
  std::string operator()(const T& self) const {
    return std::to_string(self);
  }
};

template <class D, class O>
struct ConversionConvention {
  static constexpr bool is_direct = false;
  using dispatch_type = D;
  using overload_type = O;
  using access_type = pro::implicit_conversion_access;
};

struct Stringable {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<
      ConversionConvention<pro::implicit_conversion_dispatch, int() const>,
      ConversionConvention<ToStringDispatch, std::string() const>>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};

int main() {
  pro::proxy<Stringable> p = pro::make_proxy<Stringable>(123);
  int value = *p;
  std::string str = *p;
  std::cout << value << "\n"; // Prints "123"
  std::cout << str << "\n";   // Prints "123"
}
```

## See Also

- [class `implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md)
- [class `explicit_conversion_access`](../explicit_conversion_access/README.md)
- [class template `operator_access`](../operator_access/README.md)
