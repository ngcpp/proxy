# Function template `reflect` (`proxy<F>`)

> Since: 4.1.0

```cpp
template <class R>
const R& reflect(const proxy<F>& p) noexcept;
```

Acquires reflection information of the contained type of a `proxy<F>`, through a *direct* reflection.

Let `P` be the contained type of `p`. Returns a `const` reference of `R` direct-non-list-initialized with [`std::in_place_type<P>`](https://en.cppreference.com/w/cpp/utility/in_place). The behavior is undefined if `p` does not contain a value.

There shall be a reflection type `Refl` defined in `typename F::reflection_types` where

- `Refl::is_direct` is `true`, and
- `typename Refl::reflector_type` is `R`.

The reference obtained from `reflect()` may be invalidated if `p` is subsequently modified.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy<F>` is an associated class of the arguments.

To acquire an *indirect* reflection (deduced from the pointed-to type), use [`reflect`](../proxy_indirect_accessor/friend_reflect.md) on the associated [`proxy_indirect_accessor<F>`](../proxy_indirect_accessor/README.md) (i.e., on `*p`).

## Notes

`reflect` was introduced in `4.1.0` as a replacement for the deprecated [`proxy_reflect`](../proxy_reflect.md). `proxy_reflect` is a namespace-scope function, while `reflect` is a non-member function of `proxy` found only via argument-dependent lookup.

This function is useful when only metadata deduced from a type is needed. While [`invoke`](friend_invoke.md) can also retrieve type metadata, `reflect` can generate more efficient code in this context.

## Example

```cpp
#include <iostream>
#include <memory>
#include <type_traits>

#include <proxy/proxy.h>

class CopyabilityReflector {
public:
  template <class T>
  constexpr explicit CopyabilityReflector(std::in_place_type_t<T>)
      : copyable_(std::is_copy_constructible_v<T>) {}

  template <class P, class R>
  struct accessor {
    bool IsCopyable() const noexcept {
      const CopyabilityReflector& self =
          reflect<R>(static_cast<const P&>(*this));
      return self.copyable_;
    }
  };

private:
  bool copyable_;
};

struct CopyabilityAware : pro::facade_builder                           //
                          ::add_direct_reflection<CopyabilityReflector> //
                          ::build {};

int main() {
  pro::proxy<CopyabilityAware> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.IsCopyable() << "\n"; // Prints "false"

  pro::proxy<CopyabilityAware> p2 = std::make_shared<int>();
  std::cout << p2.IsCopyable() << "\n"; // Prints "true"
}
```

## See Also

- [function template `invoke` (`proxy<F>`)](friend_invoke.md)
- [alias template `basic_facade_builder::add_reflection`](../basic_facade_builder/add_reflection.md)
