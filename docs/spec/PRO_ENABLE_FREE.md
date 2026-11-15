# Macro `PRO_ENABLE_FREE`

> Header: `proxy_macros.h` and `proxy.h`  
> Since: 5.0.0

```cpp
PRO_ENABLE_FREE(name);
```

Macro `PRO_ENABLE_FREE` defines the specializations [`free_dispatch<"name">`](free_dispatch.md) and [`free_access<"name">`](free_access.md), where `"name"` is the string literal that spells `name`. `name` shall be an identifier.

The macro shall be invoked at global namespace scope, after the class templates `free_dispatch` and `free_access` are declared, and before any use of the specializations. Including `proxy.h` or importing the module `proxy.v5` declares the class templates. The same name shall not be enabled more than once in a translation unit. Each translation unit that uses the specializations shall enable the name, so a name used in several translation units is typically enabled in a header with an include guard. Since [`PRO_ENABLE_MEMBER`](PRO_ENABLE_MEMBER.md) defines specializations of other class templates, both macros can enable the same name in a translation unit.

When headers from different major versions of the Proxy library can appear in the same translation unit (for example, Proxy 4 and Proxy 5), use the major-qualified form `PRO<major>_ENABLE_FREE` (e.g., `PRO5_ENABLE_FREE`).

## Modules

When importing the module `proxy.v5`, include `proxy/proxy_macros.h` in the global module fragment, together with the standard headers that the expansion of the macro uses: `<type_traits>`, `<utility>`, and `<tuple>` when `NDEBUG` is not defined. Invoke the macro after `import proxy.v5;` at global namespace scope in an `extern "C++"` block, as in the [module example](../modules_support.md). The module does not export macros.

The `extern "C++"` block keeps the specializations from being attached to the named module (see [module ownership](https://en.cppreference.com/w/cpp/language/modules#Module_ownership)). An entity attached to a named module can only be defined in that module, so without the block the specializations could not also be defined by the other modules and translation units that enable the same name.

## Notes

`free_dispatch<"name">` is a single entity in the whole program, shared by every translation unit and every library that enables `name`. Its call operator finds the overloads of `name` by ordinary lookup at the invocation of the macro and by argument-dependent lookup where it is instantiated (see [`free_dispatch`](free_dispatch.md)). If, for the same argument types, these lookups find different overloads in different translation units, or at different points of instantiation in one translation unit, so that the call selects different functions, the program violates the one-definition rule, and no diagnostic is required. The library cannot detect this. To avoid it, declare every overload of `name` either before the invocation of the macro in a header that every translation unit enabling `name` includes, or in a namespace associated with its argument types, next to the declarations of those types.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Buffer {};

int size(const Buffer&) noexcept { return 42; }

PRO_ENABLE_FREE(size);

struct Sized
    : pro::facade_builder                                                //
      ::add_convention<pro::free_dispatch<"size">, int() const noexcept> //
      ::build {};

int main() {
  Buffer buffer;
  pro::proxy<Sized> p = &buffer;
  std::cout << size(*p) << "\n"; // Prints "42"
}
```

## See Also

- [class template `free_dispatch`](free_dispatch.md)
- [class template `free_access`](free_access.md)
- [macro `PRO_ENABLE_MEMBER`](PRO_ENABLE_MEMBER.md)
