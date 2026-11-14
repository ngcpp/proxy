# Class template `member_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

The definition of `member_dispatch` makes use of an exposition-only type *string-literal*, which is constructible from a `char` array at compile-time and can be used as a non-type template argument.

```cpp
template <string-literal Name>
struct member_dispatch;
```

Class template `member_dispatch` is a [dispatch](ProDispatch.md) type for member function call expressions. The primary template is not defined. [`PRO_ENABLE_MEMBER(name)`](PRO_ENABLE_MEMBER.md) defines the specialization `member_dispatch<"name">`, where `name` is the name of the member function. A convention of `member_dispatch<Name>` added via [`basic_facade_builder`](basic_facade_builder/add_convention.md) has the access type [`member_access<Name>`](member_access.md).

The specialization `member_dispatch<"name">` is an empty, trivially copyable and nothrow default constructible type with the following call operator:

```cpp
template <class T, class... Args>
decltype(auto) operator()(T&& self, Args&&... args) const
    noexcept(noexcept(std::forward<T>(self).name(std::forward<Args>(args)...)))
    requires(requires { std::forward<T>(self).name(std::forward<Args>(args)...); });
```

The call operator is equivalent to `return std::forward<T>(self).name(std::forward<Args>(args)...);`. It is a static member function when the implementation supports static call operators.

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_ENABLE_MEMBER(at);

struct Dictionary : pro::facade_builder //
                    ::add_convention<pro::member_dispatch<"at">,
                                     std::string(int index) const> //
                    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n"; // Prints "world"
}
```

## See Also

- [class template `member_access`](member_access.md)
- [class template `free_dispatch`](free_dispatch.md)
- [macro `PRO_ENABLE_MEMBER`](PRO_ENABLE_MEMBER.md)
