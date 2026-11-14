# Class template `free_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

The definition of `free_dispatch` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Name>
struct free_dispatch;
```

Class template `free_dispatch` is a [dispatch](ProDispatch.md) type for free function call expressions. The primary template is not defined. [`PRO_ENABLE_FREE(name)`](PRO_ENABLE_FREE.md) defines the specialization `free_dispatch<"name">`, where `name` is the name of the free function. A convention of `free_dispatch<Name>` added via [`basic_facade_builder`](basic_facade_builder/add_convention.md) has the access type [`free_access<Name>`](free_access.md).

The specialization `free_dispatch<"name">` is an empty, trivially copyable and nothrow default constructible type with the following call operator:

```cpp
template <class T, class... Args>
decltype(auto) operator()(T&& self, Args&&... args) const
    noexcept(noexcept(name(std::forward<T>(self), std::forward<Args>(args)...)))
    requires(requires { name(std::forward<T>(self), std::forward<Args>(args)...); });
```

The call operator is equivalent to `return name(std::forward<T>(self), std::forward<Args>(args)...);`. It is a static member function when the implementation supports static call operators.

The call is unqualified. Ordinary unqualified lookup starts from the call operator, which is a member of `free_dispatch<"name">`. It searches the scope of that specialization, then the namespace `pro::inline v5`, then the namespace `pro` and its inline namespaces, and finally the global namespace. It stops at the first scope that declares `name`, so a declaration of `name` in the namespace `pro` or one of its inline namespaces hides a global declaration of `name`. Ordinary lookup finds only the declarations that precede the invocation of `PRO_ENABLE_FREE`. Argument-dependent lookup is performed at instantiation and also finds functions declared later.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Square {
  int side;
};

int Area(const Square& square) noexcept { return square.side * square.side; }

PRO_ENABLE_FREE(Area);

struct Shape
    : pro::facade_builder                                                //
      ::add_convention<pro::free_dispatch<"Area">, int() const noexcept> //
      ::build {};

int main() {
  Square square{3};
  pro::proxy<Shape> p = &square;
  std::cout << Area(*p) << "\n"; // Prints "9"
}
```

## See Also

- [class template `free_access`](free_access.md)
- [class template `member_dispatch`](member_dispatch.md)
- [macro `PRO_ENABLE_FREE`](PRO_ENABLE_FREE.md)
