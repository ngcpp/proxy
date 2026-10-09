# `proxy<F, MP>::~proxy`

```cpp
~proxy() requires(F::destructibility == constraint_level::trivial)
    = default;
~proxy() noexcept(F::destructibility == constraint_level::nothrow)
    requires(F::destructibility == constraint_level::nontrivial ||
        F::destructibility == constraint_level::nothrow);
```

Destroys the `proxy` object. If the `proxy` contains a value, the contained value is also destroyed. When `F::destructibility == constraint_level::trivial`, the destructor is defaulted. It is trivial only if the metadata storage and accessor base classes also have trivial destructors.

## Example

```cpp
#include <cstdio>

#include <proxy/proxy.h>

struct AnyMovable : pro::facade_builder::build {};

struct Foo {
  ~Foo() { puts("Destroy Foo"); }
};

int main() {
  pro::proxy<AnyMovable> p = pro::make_proxy<AnyMovable, Foo>();
} // The destructor of `Foo` is called when `p` is destroyed
```
