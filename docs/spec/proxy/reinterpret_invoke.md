# Function template `reinterpret_invoke` (`proxy<F>`)

> Since: 4.1.0

```cpp
template <class P, class D, class R, class... Args>
R reinterpret_invoke(proxy<F>& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(const proxy<F>& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(proxy<F>&& p, Args&&... args);
template <class P, class D, class R, class... Args>
R reinterpret_invoke(const proxy<F>&& p, Args&&... args);
```

Invokes a dispatch on the value contained in a `proxy<F>`, reinterpreting the underlying storage as a caller-specified pointer type `P`. `D` is a dispatch type, `R` is the return type, and `Args...` are the argument types forwarded to the dispatch.

Let `ptr` be the contained value of `p`, with the same cv ref-qualifiers as `p`. **The behavior is undefined unless `p` contains a value whose type is `P`.** Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), ptr, std::forward<Args>(args)...)`.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy<F>` is an associated class of the arguments. To reinterpret-invoke through the pointed-to value, use [`reinterpret_invoke`](../proxy_indirect_accessor/reinterpret_invoke.md) on the associated [`proxy_indirect_accessor<F>`](../proxy_indirect_accessor/README.md) (i.e., on `*p`).

## Notes

`reinterpret_invoke` is a low-level primitive. In contrast to [`invoke`](invoke.md), it performs **no type erasure**: it neither consults the runtime metadata of the `proxy` nor requires `D` to correspond to a convention registered in `typename F::convention_types`. Instead, the caller names the exact contained pointer type `P`, and the implementation reinterprets the `proxy`'s storage as `P` directly. This avoids the indirection of a virtual call, at the cost of requiring the contained type to be known statically — supplying a `P` that does not match the contained value is undefined behavior.

For ordinary use, prefer an [`accessor`](../ProAccessible.md) or [`invoke`](invoke.md), which are type-erased and do not require the caller to know the contained type. `reinterpret_invoke` is intended for advanced scenarios, such as implementing custom dispatch types or [accessors](../ProAccessible.md), where the concrete pointer type is already known.

## Example

```cpp
#include <iostream>
#include <memory>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemUseCount, use_count);

struct SharedAware
    : pro::facade_builder                                          //
      ::add_direct_convention<MemUseCount, long() const noexcept>  //
      ::build {};

int main() {
  pro::proxy<SharedAware> p =
      std::make_shared<int>(123); // The contained pointer type is shared_ptr

  // Type-erased invocation via the runtime metadata of `p`:
  std::cout << invoke<MemUseCount, long() const noexcept>(p) << "\n"; // "1"

  // Low-level invocation: we already know `p` holds a `std::shared_ptr<int>`,
  // so reinterpret the storage and dispatch directly, with no virtual call:
  std::cout << reinterpret_invoke<std::shared_ptr<int>, MemUseCount, long>(p)
            << "\n"; // Also prints "1"
}
```

## See Also

- [function template `invoke` (`proxy<F>`)](invoke.md)
- [named requirements *ProDispatch*](../ProDispatch.md)
