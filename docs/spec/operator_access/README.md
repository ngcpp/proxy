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

Class template `operator_access` is an access type that defines operator functions for [`proxy`](../proxy/README.md). It meets the [*ProxyAccess* requirements](../ProxyAccess.md) of applicable types. `Sign` represents the sign of operator (SOP) as a string literal (e.g., `"+"` for operator `+`). `Rhs` specifies whether the `proxy` operand is on the right-hand side of a binary operator.

`operator_access<Sign, Rhs>` is defined for the same SOPs as [`operator_dispatch<Sign, Rhs>`](../operator_dispatch/README.md), and instantiating it with any other `Sign` and `Rhs` is ill-formed. It is the access type of the operations of `operator_dispatch<Sign, Rhs>` added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md). An operation can also name it as its access type with any other [dispatch](../ProxyDispatch.md) type. The direct, or the indirect, operations whose access type is `operator_access<Sign, Rhs>` share an accessor (see [*ProxyAccess* requirements](../ProxyAccess.md)), so their overloads participate in overload resolution together when the operator expression is evaluated.

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

template <class D, class S>
struct CallOperation {
  static constexpr bool is_direct = false;
  using dispatch_type = D;
  using signature_type = S;
  using access_type = pro::operator_access<"()">;
};

struct Runnable {
  using super_types = std::tuple<>;
  using operation_types =
      std::tuple<CallOperation<pro::operator_dispatch<"()">, void(int)>,
                 CallOperation<RunDispatch, void()>>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::lifetime_requirement::none;
  static constexpr auto relocatability = pro::lifetime_requirement::trivial;
  static constexpr auto destructibility = pro::lifetime_requirement::nothrow;
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
