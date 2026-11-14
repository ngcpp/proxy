# Class template `free_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

The definition of `free_access` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Name>
struct free_access;
```

Class template `free_access` is an access type that defines free functions for [`proxy`](proxy/README.md). It meets the [*ProAccess* requirements](ProAccess.md) of applicable types. The primary template is not defined. [`PRO_ENABLE_FREE(name)`](PRO_ENABLE_FREE.md) defines the specialization `free_access<"name">`, whose functions are named `name`.

`free_access<Name>` is the access type of the conventions of [`free_dispatch<Name>`](free_dispatch.md) added via [`basic_facade_builder`](basic_facade_builder/add_convention.md). A convention can also name it as its access type with any other [dispatch](ProDispatch.md) type, and a reflection can name it with any reflector type. The direct, or the indirect, conventions and reflections whose access type is `free_access<Name>` share an accessor (see [*ProAccess* requirements](ProAccess.md)), so their functions participate in overload resolution together.

## Member Template `accessor`

```cpp
// (1)
template <class Self, class... Ds>
struct accessor {
  accessor() = delete;
};

// (2)
template <class Self, class... Ds>
    requires(sizeof...(Ds) > 1u && (std::is_constructible_v<accessor<Self, Ds>> && ...))
struct accessor<Self, Ds...> : accessor<Self, Ds>... {};

// (3)
template <class Self, class D, class R, class... Args>
struct accessor<Self, proxy_operation<D, R(Args...) cv ref noex>> {
  friend R name(Self cv <ref ? ref : &> self, Args... args) noex;
};

// (4)
template <class Self, class M>
struct accessor<Self, proxy_reflection<M>> {
  friend const M& name(const Self& self) noexcept;
};
```

Each type in `Ds` is a *descriptor* (see [*ProAccess* requirements](ProAccess.md)), `cv ref noex` ranges over the qualifiers supported by [*ProOverload*](ProOverload.md), and `name` is the name passed to `PRO_ENABLE_FREE`. The functions are friends defined in the accessor, which [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) finds when `Self` is an associated class of the arguments.

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Ds)` is greater than `1`, and `accessor<Self, Ds>...` are default-constructible, inherits all `accessor<Self, Ds>...` types. The program is ill-formed when two of their functions have the same parameter types, such as the function for a reflection and the function for an operation with an overload type `R() const`.

`(3)` When `sizeof...(Ds)` is `1` and the only type in `Ds` is [`proxy_operation`](proxy_operation.md)`<D, R(Args...) cv ref noex>`, provides a function `name(Self cv <ref ? ref : &>, Args...)` with the same *noex* specifier as the overload type. It is equivalent to `return invoke<D, R(Args...) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(self), std::forward<Args>(args)...)`. `D` need not be `free_dispatch<Name>`.

`(4)` When `sizeof...(Ds)` is `1` and the only type in `Ds` is [`proxy_reflection`](proxy_reflection.md)`<M>`, provides a function `name(const Self&)`. It is equivalent to `return reflect<M>(self)`.

## Example

```cpp
#include <iostream>
#include <tuple>

#include <proxy/proxy.h>

struct Square {
  int side;
  int Area() const noexcept { return side * side; }
};

PRO_ENABLE_MEMBER(Area);
PRO_ENABLE_FREE(Area);

struct AreaConvention {
  static constexpr bool is_direct = false;
  using dispatch_type = pro::member_dispatch<"Area">;
  using overload_type = int() const noexcept;
  using access_type = pro::free_access<"Area">;
};

struct Shape {
  using super_types = std::tuple<>;
  using convention_types = std::tuple<AreaConvention>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::trivial;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};

int main() {
  Square square{3};
  pro::proxy<Shape> p = &square;
  std::cout << Area(*p) << "\n"; // Prints "9"
}
```

## See Also

- [class template `free_dispatch`](free_dispatch.md)
- [class template `member_access`](member_access.md)
- [macro `PRO_ENABLE_FREE`](PRO_ENABLE_FREE.md)
