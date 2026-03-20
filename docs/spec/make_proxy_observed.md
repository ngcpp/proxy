# Function template `make_proxy_observed`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 4.1.0

The definition of `make_proxy_observed` makes use of an exposition-only class template *observer-ptr*. `observer-ptr<T>` contains a raw pointer to an object of type `T`, and provides `operator*` for access with the same qualifiers.

```cpp
template <facade F, class T>
proxy<F> make_proxy_observed(T& value) noexcept;
```

Creates a `proxy<F>` object containing a value `p` of type `observer-ptr<T>`, where `*p` is direct-non-list-initialized with `std::addressof(value)`. If [`proxiable_target<T, F>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy` object.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Printable : pro::facade_builder //
                   ::add_convention<pro::operator_dispatch<"<<", true>,
                                    std::ostream&(std::ostream&) const> //
                   ::build {};

int main() {
  int val = 123;
  pro::proxy<Printable> p = pro::make_proxy_observed<Printable>(val);

  // Prints "123"
  std::cout << *p << "\n";

  val = 456;

  // Prints "456"
  std::cout << *p << "\n";
}
```

## See Also

- [concept `proxiable_target`](proxiable_target.md)
- [function template `make_proxy_view`](make_proxy_view.md)