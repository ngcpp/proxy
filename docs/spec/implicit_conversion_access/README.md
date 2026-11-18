# Class `implicit_conversion_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
struct implicit_conversion_access;
```

Class `implicit_conversion_access` is an access type that defines implicit conversion functions for [`proxy`](../proxy/README.md). It meets the [*ProxyAccess* requirements](../ProxyAccess.md) of applicable types.

`implicit_conversion_access` is the access type of the operations of [`implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md) added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md). An operation can also name it as its access type with any other [dispatch](../ProxyDispatch.md) type that converts a `proxy` to the return types of its signatures. The direct, or the indirect, operations whose access type is `implicit_conversion_access` share an accessor (see [*ProxyAccess* requirements](../ProxyAccess.md)), so the conversions of all of them are available together.

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

template <class D, class S>
struct ConversionOperation {
  static constexpr bool is_direct = false;
  using dispatch_type = D;
  using signature_type = S;
  using access_type = pro::implicit_conversion_access;
};

struct Stringable {
  using super_types = std::tuple<>;
  using operation_types = std::tuple<
      ConversionOperation<pro::implicit_conversion_dispatch, int() const>,
      ConversionOperation<ToStringDispatch, std::string() const>>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::lifetime_requirement::none;
  static constexpr auto relocatability = pro::lifetime_requirement::trivial;
  static constexpr auto destructibility = pro::lifetime_requirement::nothrow;
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
