# Class `explicit_conversion_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
struct explicit_conversion_access;
```

Class `explicit_conversion_access` is an access type that defines explicit conversion functions for [`proxy`](../proxy/README.md). It meets the [*ProAccess* requirements](../ProAccess.md) of applicable types.

`explicit_conversion_access` is the access type of the conventions of [`explicit_conversion_dispatch`](../explicit_conversion_dispatch/README.md) added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md). A convention can also name it as its access type with any other [dispatch](../ProDispatch.md) type that converts a `proxy` to the return types of its overloads. The direct, or the indirect, conventions whose access type is `explicit_conversion_access` share an accessor (see [*ProAccess* requirements](../ProAccess.md)), so the conversions of all of them are available together.

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

struct ToStringConvention {
  static constexpr bool is_direct = false;
  using dispatch_type = ToStringDispatch;
  using overload_type = std::string() const;
  using access_type = pro::explicit_conversion_access;
};

struct Stringable {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<ToStringConvention>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};

int main() {
  pro::proxy<Stringable> p = pro::make_proxy<Stringable>(123);
  std::cout << static_cast<std::string>(*p) << "\n"; // Prints "123"
}
```

## See Also

- [class `explicit_conversion_dispatch`](../explicit_conversion_dispatch/README.md)
- [class `implicit_conversion_access`](../implicit_conversion_access/README.md)
- [class template `operator_access`](../operator_access/README.md)
