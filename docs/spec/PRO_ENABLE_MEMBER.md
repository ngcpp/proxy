# Macro `PRO_ENABLE_MEMBER`

> Header: `proxy_macros.h` and `proxy.h`  
> Since: 5.0.0

```cpp
PRO_ENABLE_MEMBER(name);
```

Macro `PRO_ENABLE_MEMBER` defines the specializations [`member_dispatch<"name">`](member_dispatch.md) and [`member_access<"name">`](member_access.md), where `"name"` is the string literal that spells `name`. `name` shall be an identifier.

The macro shall be invoked at global namespace scope, after the class templates `member_dispatch` and `member_access` are declared, and before any use of the specializations. Including `proxy.h` or importing the module `proxy.v5` declares the class templates. The same name shall not be enabled more than once in a translation unit. Each translation unit that uses the specializations shall enable the name, so a name used in several translation units is typically enabled in a header with an include guard. Since [`PRO_ENABLE_FREE`](PRO_ENABLE_FREE.md) defines specializations of other class templates, both macros can enable the same name in a translation unit.

When headers from different major versions of the Proxy library can appear in the same translation unit (for example, Proxy 4 and Proxy 5), use the major-qualified form `PRO<major>_ENABLE_MEMBER` (e.g., `PRO5_ENABLE_MEMBER`).

## Modules

When importing the module `proxy.v5`, include `proxy/proxy_macros.h` in the global module fragment, together with the standard headers that the expansion of the macro uses: `<type_traits>`, `<utility>`, and `<tuple>` when `NDEBUG` is not defined. Invoke the macro after `import proxy.v5;` at global namespace scope in an `extern "C++"` block, as in the [module example](../modules_support.md). The module does not export macros.

The `extern "C++"` block keeps the specializations from being attached to the named module (see [module ownership](https://en.cppreference.com/w/cpp/language/modules#Module_ownership)). An entity attached to a named module can only be defined in that module, so without the block the specializations could not also be defined by the other modules and translation units that enable the same name.

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
