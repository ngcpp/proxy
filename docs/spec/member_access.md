# Class template `member_access`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

The definition of `member_access` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Name>
struct member_access;
```

Class template `member_access` is an access type that defines member functions for [`proxy`](proxy/README.md). It meets the [*ProAccess* requirements](ProAccess.md) of applicable types. The primary template is not defined. [`PRO_ENABLE_MEMBER(name)`](PRO_ENABLE_MEMBER.md) defines the specialization `member_access<"name">`, whose member functions are named `name`.

`member_access<Name>` is the access type of the conventions of [`member_dispatch<Name>`](member_dispatch.md) added via [`basic_facade_builder`](basic_facade_builder/add_convention.md). A convention can also name it as its access type with any other [dispatch](ProDispatch.md) type, and a reflection can name it with any reflector type. The direct, or the indirect, conventions and reflections whose access type is `member_access<Name>` share an accessor (see [*ProAccess* requirements](ProAccess.md)), so their member functions participate in overload resolution together.

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
struct accessor<Self, Ds...> : accessor<Self, Ds>... {
  using accessor<Self, Ds>::name...;
};

// (3)
template <class Self, class D, class R, class... Args>
struct accessor<Self, proxy_operation<D, R(Args...) cv ref noex>> {
  R name(Args... args) cv ref noex;
};

// (4)
template <class Self, class M>
struct accessor<Self, proxy_reflection<M>> {
  const M& name() const noexcept;
};
```

Each type in `Ds` is a *descriptor* (see [*ProAccess* requirements](ProAccess.md)), `cv ref noex` ranges over the qualifiers supported by [*ProOverload*](ProOverload.md), and `name` is the name passed to `PRO_ENABLE_MEMBER`.

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Ds)` is greater than `1`, and `accessor<Self, Ds>...` are default-constructible, inherits all `accessor<Self, Ds>...` types and `using` their `name`. A call of `name` is ambiguous when two of these functions have the same parameter types and the same *cv ref* qualifiers, such as the function for a reflection and the function for an operation with an overload type `R() const`.

`(3)` When `sizeof...(Ds)` is `1` and the only type in `Ds` is [`proxy_operation`](proxy_operation.md)`<D, R(Args...) cv ref noex>`, provides a member function `name(Args...)` with the same *cv ref noex* specifiers as the overload type. `accessor::name(Args...)` is equivalent to `return invoke<D, R(Args...) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this), std::forward<Args>(args)...)`. `D` need not be `member_dispatch<Name>`.

`(4)` When `sizeof...(Ds)` is `1` and the only type in `Ds` is [`proxy_reflection`](proxy_reflection.md)`<M>`, provides a member function `name()`. `accessor::name()` is equivalent to `return reflect<M>(static_cast<const Self&>(*this))`.

## Example

```cpp
#include <iostream>
#include <tuple>

#include <proxy/proxy.h>

struct Square {
  int side;
};

int Area(const Square& square) noexcept { return square.side * square.side; }

PRO_ENABLE_FREE(Area);
PRO_ENABLE_MEMBER(Area);

struct AreaConvention {
  static constexpr bool is_direct = false;
  using dispatch_type = pro::free_dispatch<"Area">;
  using overload_type = int() const noexcept;
  using access_type = pro::member_access<"Area">;
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
  std::cout << p->Area() << "\n"; // Prints "9"
}
```

## See Also

- [class template `member_dispatch`](member_dispatch.md)
- [class template `free_access`](free_access.md)
- [macro `PRO_ENABLE_MEMBER`](PRO_ENABLE_MEMBER.md)
