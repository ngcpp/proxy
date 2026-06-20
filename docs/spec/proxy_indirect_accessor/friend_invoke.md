# Function template `invoke` (`proxy_indirect_accessor<F>`)

> Since: 4.1.0

```cpp
template <class D, class O, class... Args>
return-type-of<O> invoke(proxy_indirect_accessor<F>& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(const proxy_indirect_accessor<F>& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(proxy_indirect_accessor<F>&& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(const proxy_indirect_accessor<F>&& p, Args&&... args);
```

Invokes a `proxy_indirect_accessor<F>` with a specified dispatch type `D`, an overload type `O`, and arguments, through an *indirect* convention. Let `Args2...` be the argument types of `O`, `R` be the return type of `O`. `return-type-of<O>` is `R`.

Let `ptr` be the contained value of the `proxy` object associated to `p` with the same cv ref-qualifiers. Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), *ptr, static_cast<Args2>(args)...)`.

There shall be a convention type `Conv` defined in `typename F::convention_types` where

- `Conv::is_direct` is `false`, and
- `typename Conv::dispatch_type` is `D`, and
- there shall be an overload type `O1` defined in `typename Conv::overload_types` where [`substituted-overload`](../ProOverload.md)`<O1, F>` is `O`.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` is an associated class of the arguments.

A `proxy_indirect_accessor<F>` is obtained by dereferencing a [`proxy<F>`](../proxy/README.md) (i.e., `*p`). To invoke a *direct* convention, use [`invoke`](../proxy/friend_invoke.md) on the [`proxy<F>`](../proxy/README.md) itself.

## Notes

`invoke` was introduced in `4.1.0` as a replacement for the deprecated [`proxy_invoke`](../proxy_invoke.md). `proxy_invoke` is a namespace-scope function, while `invoke` is a non-member function of `proxy_indirect_accessor` found only via argument-dependent lookup.

It is generally not recommended to call `invoke` directly. Using an [`accessor`](../ProAccessible.md) is usually a better option with easier and more descriptive syntax.

## Example

```cpp
#include <iostream>
#include <string>

#include <proxy/proxy.h>

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder                                 //
                    ::add_convention<FreeToString, std::string() const> //
                    ::build {};

int main() {
  int a = 123;
  pro::proxy<Stringable> p = &a;
  std::cout << ToString(*p) << "\n"; // Invokes with accessor, prints: "123"
  std::cout << invoke<FreeToString, std::string() const>(*p)
            << "\n"; // Invokes with the non-member invoke, also prints: "123"
}
```

## See Also

- [function template `reflect` (`proxy_indirect_accessor<F>`)](friend_reflect.md)
- [function template `reinterpret_invoke` (`proxy_indirect_accessor<F>`)](friend_reinterpret_invoke.md)
