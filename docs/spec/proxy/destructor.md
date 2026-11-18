# `proxy<F, MP>::~proxy`

```cpp
~proxy() requires(F::destructibility == lifetime_requirement::trivial)
    = default;
~proxy() noexcept(F::destructibility == lifetime_requirement::nothrow)
    requires(F::destructibility == lifetime_requirement::supported ||
        F::destructibility == lifetime_requirement::nothrow);
~proxy() requires(F::destructibility == lifetime_requirement::none)
    = delete;
```

Destroys the `proxy` object. If the `proxy` contains a value, the contained value is also destroyed. When `F::destructibility == lifetime_requirement::trivial`, the destructor is defaulted. It is trivial only if the metadata storage and accessor base classes also have trivial destructors. When `F::destructibility == lifetime_requirement::none`, the destructor is deleted, so a variable or a function return value of this `proxy` type is ill-formed. Such a `proxy` can be created with a *new-expression*.

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
