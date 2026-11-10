# Class `implicit_conversion_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 3.2.0

```cpp
class implicit_conversion_dispatch;
```

Class `implicit_conversion_dispatch` models a [dispatch](../ProDispatch.md) type for implicit type conversion expressions. A convention of `implicit_conversion_dispatch` added via [`basic_facade_builder`](../basic_facade_builder/add_convention.md) has the access type [`implicit_conversion_access`](../implicit_conversion_access/README.md).

*Since 5.0.0*: the accessibility is provided by `implicit_conversion_access`. Previously, `implicit_conversion_dispatch` provided the accessibility via its member template `accessor`.

## Member Functions

| Name                             | Description                                         |
| -------------------------------- | --------------------------------------------------- |
| (constructor) [nothrow]          | constructs an `implicit_conversion_dispatch` object |
| [`operator()`](operator_call.md) | invokes the dispatch                                |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Runnable : pro::facade_builder                                    //
                  ::add_convention<pro::operator_dispatch<"()">, void()> //
                  ::build {};

struct CopyableRunnable
    : pro::facade_builder                               //
      ::support_copy<pro::constraint_level::nontrivial> //
      ::add_facade<Runnable>                            //
      ::add_direct_convention<pro::implicit_conversion_dispatch,
                              pro::proxy<Runnable>() const&,
                              pro::proxy<Runnable>() &&> //
      ::build {};

int main() {
  pro::proxy<CopyableRunnable> p1 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });
  auto p2 = p1; // Copy construction

  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  pro::proxy<Runnable> p3 = p2;
  std::cout << std::boolalpha << p2.has_value() << "\n"; // Prints "true"

  // Won't compile because pro::proxy<Runnable> is not copy-constructible
  // auto p4 = p3;

  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  pro::proxy<Runnable> p5 = std::move(p2);
  std::cout << p2.has_value() << "\n"; // Prints "false"
  (*p5)();                             // Prints "Lambda expression invoked"
}
```

## See Also

- [class `implicit_conversion_access`](../implicit_conversion_access/README.md)
- [class `explicit_conversion_dispatch`](../explicit_conversion_dispatch/README.md)
- [class template `operator_dispatch`](../operator_dispatch/README.md)
