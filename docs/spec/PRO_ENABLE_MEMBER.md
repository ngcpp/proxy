# Macro `PRO_ENABLE_MEMBER`

> Header: `proxy_macros.h` and `proxy.h`  
> Since: 5.0.0

```cpp
PRO_ENABLE_MEMBER(name);
```

Macro `PRO_ENABLE_MEMBER` defines the specializations [`member_dispatch<"name">`](member_dispatch.md) and [`member_access<"name">`](member_access.md), where `"name"` is `name` converted to a string literal. `name` shall be an identifier. Qualified names, template argument lists and other expressions are not accepted.

The macro shall be invoked at global namespace scope, after `member_dispatch` and `member_access` are declared by including `proxy.h` or importing the module, and before the specializations are used. A name can be enabled at most once in a translation unit. To use the specializations in several translation units, invoke the macro in a header with an include guard. [`PRO_ENABLE_FREE`](PRO_ENABLE_FREE.md) defines specializations of other class templates, so both macros can enable the same name.

A convention of `member_dispatch<"name">` added via [`basic_facade_builder`](basic_facade_builder/add_convention.md) has the access type `member_access<"name">`. A convention can also combine either specialization with other dispatch or access types. For example, a convention with the dispatch type [`free_dispatch<"name">`](free_dispatch.md) and the access type `member_access<"name">` calls a free function through a member function of `proxy`.

When headers from different major versions of the Proxy library can appear in the same translation unit (for example, Proxy 4 and Proxy 5), use the major-qualified form `PRO<major>_ENABLE_MEMBER` (e.g., `PRO5_ENABLE_MEMBER`).

## Modules

When importing the module `proxy.v5`, include `proxy/proxy_macros.h` in the global module fragment, together with `<type_traits>` and `<utility>`, and `<tuple>` when `NDEBUG` is not defined, which the expansion of the macro uses. Invoke the macro after `import proxy.v5;` at global namespace scope in an `extern "C++"` block, as in the [module example](../modules_support.md). The module does not export macros.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Buffer {
  int size() const noexcept { return 42; }
};

PRO_ENABLE_MEMBER(size);

struct Sized
    : pro::facade_builder                                                  //
      ::add_convention<pro::member_dispatch<"size">, int() const noexcept> //
      ::build {};

int main() {
  Buffer buffer;
  pro::proxy<Sized> p = &buffer;
  std::cout << p->size() << "\n"; // Prints "42"
}
```

## See Also

- [class template `member_dispatch`](member_dispatch.md)
- [class template `member_access`](member_access.md)
- [macro `PRO_ENABLE_FREE`](PRO_ENABLE_FREE.md)
