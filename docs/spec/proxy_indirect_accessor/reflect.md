# Function template `reflect` (`proxy_indirect_accessor<F>`)

> Since: 4.1.0

```cpp
template <class R>
const R& reflect(const proxy_indirect_accessor<F>& p) noexcept;
```

Acquires reflection information of the contained type of the associated `proxy`, through an *indirect* reflection.

Let `P` be the contained type of the `proxy` object associated to `p`. Returns a `const` reference of `R` direct-non-list-initialized with [`std::in_place_type<typename std::pointer_traits<P>::element_type>`](https://en.cppreference.com/w/cpp/utility/in_place).

There shall be a reflection type `Refl` defined in `typename F::reflection_types` where

- `Refl::is_direct` is `false`, and
- `typename Refl::reflector_type` is `R`.

The reference obtained from `reflect()` may be invalidated if the associated `proxy` is subsequently modified.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` is an associated class of the arguments.

A `proxy_indirect_accessor<F>` is obtained by dereferencing a [`proxy<F>`](../proxy/README.md) (i.e., `*p`). To acquire a *direct* reflection (deduced from the pointer type), use [`reflect`](../proxy/reflect.md) on the [`proxy<F>`](../proxy/README.md) itself.

## Notes

`reflect` was introduced in `4.1.0` as a replacement for the deprecated [`proxy_reflect`](../proxy_reflect.md). `proxy_reflect` is a namespace-scope function, while `reflect` is a non-member function of `proxy_indirect_accessor` found only via argument-dependent lookup.

This function is useful when only metadata deduced from a type is needed. While [`invoke`](invoke.md) can also retrieve type metadata, `reflect` can generate more efficient code in this context.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

class LayoutReflector {
public:
  template <class T>
  constexpr explicit LayoutReflector(std::in_place_type_t<T>)
      : Size(sizeof(T)), Align(alignof(T)) {}

  template <class P, class R>
  struct accessor {
    friend std::size_t SizeOf(const P& self) noexcept {
      const LayoutReflector& refl = reflect<R>(self);
      return refl.Size;
    }
  };

  std::size_t Size, Align;
};

struct LayoutAware : pro::facade_builder                          //
                     ::add_indirect_reflection<LayoutReflector>   //
                     ::build {};

int main() {
  int a = 123;
  pro::proxy<LayoutAware> p = &a;
  std::cout << SizeOf(*p) << "\n"; // Prints sizeof(int), the pointed-to type
}
```

## See Also

- [function template `invoke` (`proxy_indirect_accessor<F>`)](invoke.md)
- [alias template `basic_facade_builder::add_reflection`](../basic_facade_builder/add_reflection.md)
