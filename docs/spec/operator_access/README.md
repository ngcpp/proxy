# Class template `operator_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

The definition of `operator_access` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Sign, bool Rhs = false>
struct operator_access;
```

Class template `operator_access` is an access type that defines operator functions for [`proxy`](../proxy/README.md). It meets the [*ProAccess* requirements](../ProAccess.md) of applicable types. `Sign` represents the sign of operator (SOP) as a string literal (e.g., `"+"` for operator `+`). `Rhs` specifies whether the `proxy` operand is on the right-hand side of a binary operator.

`operator_access<Sign, Rhs>` is defined for the same SOPs as [`operator_dispatch<Sign, Rhs>`](../operator_dispatch/README.md), and is the access type of the conventions of `operator_dispatch<Sign, Rhs>` added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md). A convention can also name it as its access type with any other [dispatch](../ProDispatch.md) type. The direct, or the indirect, conventions whose access type is `operator_access<Sign, Rhs>` share an accessor (see [*ProAccess* requirements](../ProAccess.md)), so their overloads participate in overload resolution together when the operator expression is evaluated.

## Supported SOPs

`operator_access` supports the same 37 SOPs as [`operator_dispatch`](../operator_dispatch/README.md#supported-sops). The expressions available for each specialization of `operator_access` are listed in the [specializations of `operator_dispatch`](../operator_dispatch/README.md#specializations).

## Member Types

| Name                      | Description                       |
| ------------------------- | --------------------------------- |
| [`accessor`](accessor.md) | defines functions for `proxy` |

## Example

```cpp
#include <iostream>
#include <tuple>

#include <proxy/proxy.h>

struct RunDispatch {
  template <class T>
  void operator()(T& self) const {
    self.Run();
  }
};

template <class D, class O>
struct CallConvention {
  static constexpr bool is_direct = false;
  using dispatch_type = D;
  using overload_type = O;
  using access_type = pro::operator_access<"()">;
};

struct Runnable {
  using super_types = std::tuple<>;
  using convention_types =
      std::tuple<CallConvention<pro::operator_dispatch<"()">, void(int)>,
                 CallConvention<RunDispatch, void()>>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};

struct Job {
  void operator()(int n) { std::cout << "Called with " << n << "\n"; }
  void Run() { std::cout << "Run\n"; }
};

int main() {
  Job job;
  pro::proxy<Runnable> p = &job;
  (*p)(123); // Prints "Called with 123"
  (*p)();    // Prints "Run"
}
```

## See Also

- [class template `operator_dispatch`](../operator_dispatch/README.md)
- [class `explicit_conversion_access`](../explicit_conversion_access/README.md)
- [class `implicit_conversion_access`](../implicit_conversion_access/README.md)
