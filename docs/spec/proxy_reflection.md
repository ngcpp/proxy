# Class template `proxy_reflection`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
template <class M>
struct proxy_reflection {};
```

Class template `proxy_reflection` is a *descriptor* of a reflection of a [`proxy`](proxy/README.md) whose reflector type is `M`. Descriptors are the template arguments following `Self` of the member template `accessor` of an access type (see [*ProAccess* requirements](ProAccess.md)), telling the access type which operations and reflections its accessor provides accessibility for.

For a reflection type `R` of a [facade](facade.md) type `F` or of any super of `F`, the descriptor of `R` in [`proxy<F, MP>`](proxy/README.md) or [`proxy_indirect_accessor<F, MP>`](proxy_indirect_accessor/README.md) is `proxy_reflection<typename R::reflector_type>`. Within an accessor, the described reflection is acquired by calling [`reflect<M>`](proxy/friend_reflect.md) with the `Self` object. The access type of a reflection added by [`basic_facade_builder`](basic_facade_builder/add_reflection.md) is the default access type of its reflector type.

## Example

```cpp
#include <iostream>
#include <tuple>
#include <typeinfo>

#include <proxy/proxy.h>

struct TypeNameAccess {
  template <class Self, class... Ds>
  struct accessor {
    accessor() = delete;
  };
  template <class Self, class M>
  struct accessor<Self, pro::proxy_reflection<M>> {
    friend const char* TypeName(const Self& self) noexcept {
      return reflect<M>(self).Info->name();
    }
  };
};

struct TypeInfoReflector {
  TypeInfoReflector() = default;
  template <class T>
  constexpr explicit TypeInfoReflector(std::in_place_type_t<T>) noexcept
      : Info(&typeid(T)) {}

  const std::type_info* Info;
};

template <bool IsDirect>
struct TypeNameReflection {
  static constexpr bool is_direct = IsDirect;
  using reflector_type = TypeInfoReflector;
  using access_type = TypeNameAccess;
};

struct TypeNameAware {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<>;
  using reflection_types =
      std::tuple<TypeNameReflection<true>, TypeNameReflection<false>>;
  static constexpr std::size_t max_size = sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::trivial;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::trivial;
};

int main() {
  int a = 123;
  pro::proxy<TypeNameAware> p = &a;
  std::cout << TypeName(p) << "\n";  // Prints the name of int*
  std::cout << TypeName(*p) << "\n"; // Prints the name of int
}
```

## See Also

- [class template `proxy_operation`](proxy_operation.md)
- [named requirements *ProAccess*](ProAccess.md)
- [named requirements *ProBasicReflection*](ProBasicReflection.md)
