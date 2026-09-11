# Alias template `weak_proxy`<br />Class template `weak_facade`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.3.0

```cpp
template <facade F>
struct weak_facade;

template <facade F>
using weak_proxy = proxy<weak_facade<F>>;
```

`weak_proxy<F>` is a non-owning handle that participates in the weak ownership model of an object that (when alive) models [`proxiable_target<T, F>`](proxiable_target.md). It is analogous to `std::weak_ptr` relative to `std::shared_ptr`: it never extends the lifetime of the underlying object, but it can attempt to produce a (strong) `proxy<F>` via `lock()`.

`weak_facade<F>` adapts the original [facade](facade.md) `F` so that:

* A `lock()` member (direct convention) is provided, returning a `proxy<F>` that contains a value if and only if the referenced object is still alive at the time of the call.
* Each super of `F` is adapted in the same way, so that "weak-ness" is preserved when converting to a `proxy` of a super: a `weak_proxy<F>` converts to a `weak_proxy<G>` for every super `G` of `F`.
* No reflections from `F` are preserved.

## Member Types of `weak_facade`

| Name                               | Description |
| ---------------------------------- | ----------- |
| `super_types`<br />*(since 5.0.0)* | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::super_types`. Specifically, for each super `S` in `typename F::super_types`, `weak_facade<S>` is included. |
| `convention_types`                 | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains a single direct convention whose dispatch type denotes the member function `lock` and whose overload has signature `proxy<F>() const noexcept`. Calling this overload attempts to obtain a strong `proxy<F>`; it returns an empty `proxy<F>` if the object has expired. All conventions from `F` are discarded. |
| `reflection_types`                 | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains no types. |

## Member Constants of `weak_facade`

| Name                                   | Definition           |
| -------------------------------------- | -------------------- |
| `max_size` [static] [constexpr]        | `F::max_size`        |
| `max_align` [static] [constexpr]       | `F::max_align`       |
| `copyability` [static] [constexpr]     | `F::copyability`     |
| `relocatability` [static] [constexpr]  | `F::relocatability`  |
| `destructibility` [static] [constexpr] | `F::destructibility` |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Formattable : pro::facade_builder              //
                     ::add_skill<pro::skills::format> //
                     ::build {};

int main() {
  std::shared_ptr<int> val = std::make_shared<int>(123);
  pro::weak_proxy<Formattable> wp = std::weak_ptr{val};
  pro::proxy<Formattable> p = wp.lock();
  std::cout << std::boolalpha << p.has_value() << "\n"; // Prints "true"
  std::cout << std::format("{}\n", *p);                 // Prints "123"

  p.reset();
  val.reset();
  p = wp.lock();
  std::cout << p.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [alias template `skills::as_weak`](skills_as_weak.md)
- [function template `make_proxy_shared`](make_proxy_shared.md)
