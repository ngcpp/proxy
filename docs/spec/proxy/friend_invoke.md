# Function template `invoke` (`proxy<F>`)

> Since: 4.1.0

```cpp
template <class D, class O, class... Args>
return-type-of<O> invoke(proxy<F>& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(const proxy<F>& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(proxy<F>&& p, Args&&... args);
template <class D, class O, class... Args>
return-type-of<O> invoke(const proxy<F>&& p, Args&&... args);
```

Invokes a `proxy<F>` with a specified dispatch type `D`, an overload type `O`, and arguments, through a *direct* convention. Let `Args2...` be the argument types of `O`, `R` be the return type of `O`. `return-type-of<O>` is `R`.

Let `ptr` be the contained value of `p` with the same cv ref-qualifiers. Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), ptr, static_cast<Args2>(args)...)`. The behavior is undefined if `p` does not contain a value.

There shall be a convention type `Conv` defined in `typename F::convention_types` where

- `Conv::is_direct` is `true`, and
- `typename Conv::dispatch_type` is `D`, and
- there shall be an overload type `O1` defined in `typename Conv::overload_types` where [`substituted-overload`](../ProOverload.md)`<O1, F>` is `O`.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy<F>` is an associated class of the arguments.

To invoke an *indirect* convention, use [`invoke`](../proxy_indirect_accessor/friend_invoke.md) on the associated [`proxy_indirect_accessor<F>`](../proxy_indirect_accessor/README.md) (i.e., on `*p`).

## Notes

`invoke` was introduced in `4.1.0` as a replacement for the deprecated [`proxy_invoke`](../proxy_invoke.md). `proxy_invoke` is a namespace-scope function, while `invoke` is a non-member function of `proxy` found only via argument-dependent lookup.

It is generally not recommended to call `invoke` directly. Using an [`accessor`](../ProAccessible.md) is usually a better option with easier and more descriptive syntax.

## Example

```cpp
#include <iostream>
#include <memory>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemUseCount, use_count);

// A direct convention operates on the contained pointer itself.
struct SharedAware
    : pro::facade_builder                                          //
      ::add_direct_convention<MemUseCount, long() const noexcept>  //
      ::build {};

int main() {
  pro::proxy<SharedAware> p = std::make_shared<int>(123);
  std::cout << p.use_count() << "\n"; // Invokes with accessor, prints: "1"
  std::cout << invoke<MemUseCount, long() const noexcept>(p)
            << "\n"; // Invokes with the non-member invoke, also prints: "1"
}
```

## See Also

- [function template `reflect` (`proxy<F>`)](friend_reflect.md)
- [function template `reinterpret_invoke` (`proxy<F>`)](friend_reinterpret_invoke.md)
