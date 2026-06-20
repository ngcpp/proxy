# Function template `reinterpret_invoke` (`proxy_indirect_accessor<F>`)

> Since: 4.1.0

```cpp
template <class P, class D, class R, class... Args>
R reinterpret_invoke(proxy_indirect_accessor<F>& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(const proxy_indirect_accessor<F>& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(proxy_indirect_accessor<F>&& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(const proxy_indirect_accessor<F>&& p, Args&&... args);
```

Invokes a dispatch on the value contained in the associated `proxy`, reinterpreting the underlying storage as a caller-specified pointer type `P`. `D` is a dispatch type, `R` is the return type, and `Args...` are the argument types forwarded to the dispatch.

Let `ptr` be the contained value of the `proxy` object associated to `p`, with the same cv ref-qualifiers as `p`. **The behavior is undefined unless the associated `proxy` contains a value whose type is `P`.** Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), *ptr, std::forward<Args>(args)...)`.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` is an associated class of the arguments. To reinterpret-invoke on the contained pointer itself, use [`reinterpret_invoke`](../proxy/friend_reinterpret_invoke.md) on the [`proxy<F>`](../proxy/README.md).

## Notes

`reinterpret_invoke` is a low-level primitive. In contrast to [`invoke`](friend_invoke.md), it performs **no type erasure**: it neither consults the runtime metadata of the `proxy` nor requires `D` to correspond to a convention registered in `typename F::convention_types`. Instead, the caller names the exact contained pointer type `P`, and the implementation reinterprets the `proxy`'s storage as `P` directly. This avoids the indirection of a virtual call, at the cost of requiring the contained type to be known statically. Supplying a `P` that does not match the contained value is undefined behavior.

For ordinary use, prefer an [`accessor`](../ProAccessible.md) or [`invoke`](friend_invoke.md), which are type-erased and do not require the caller to know the contained type. `reinterpret_invoke` is intended for advanced scenarios, such as implementing custom dispatch types or [accessors](../ProAccessible.md), where the concrete pointer type is already known.

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
  pro::proxy<Stringable> p = &a; // The contained pointer type is `int*`

  // Type-erased invocation via the runtime metadata of the associated proxy:
  std::cout << invoke<FreeToString, std::string() const>(*p) << "\n"; // "123"

  // Low-level invocation: we already know the proxy holds an `int*`, so
  // reinterpret the storage as `int*` and dispatch directly, with no virtual
  // call. The dispatch receives the pointed-to `int`:
  std::cout << reinterpret_invoke<int*, FreeToString, std::string>(*p)
            << "\n"; // Also prints "123"
}
```

## See Also

- [function template `invoke` (`proxy_indirect_accessor<F>`)](friend_invoke.md)
- [named requirements *ProDispatch*](../ProDispatch.md)
