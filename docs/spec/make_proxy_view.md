# Function template `make_proxy_view`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 3.3.0

The definition of `make_proxy_view` makes use of an exposition-only class template *observer-ptr*. `observer-ptr<T>` contains a raw pointer to an object of type `T`, and provides `operator*` for access with the same qualifiers.

```cpp
template <facade F, class MP = compact_metadata, class T>
proxy_view<F, MP> make_proxy_view(T& value) noexcept;
```

Creates a `proxy_view<F, MP>` object containing a value `p` of type `observer-ptr<T>`, where `p` is direct-non-list-initialized with `std::addressof(value)`. If [`proxiable_target<T, F, MP>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

*Since 5.0.0*: the [metadata policy](ProMetadataPolicy.md) of the created `proxy_view` can be named explicitly, and defaults to [`compact_metadata`](compact_metadata.md). `make_proxy_view` has a single overload and no target-type parameter, so the policy is named right after the facade.

## Return Value

The constructed `proxy_view` object.

## Example

```cpp
#include <iostream>
#include <map>
#include <string>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct ResourceDictionary
    : pro::facade_builder //
      ::add_convention<MemAt, std::string&(int index),
                       const std::string&(int index) const> //
      ::build {};

int main() {
  std::map<int, std::string> dict;
  dict[1] = "init";
  pro::proxy_view<ResourceDictionary> pv =
      pro::make_proxy_view<ResourceDictionary>(dict);
  static_assert(std::is_same_v<decltype(pv->at(1)), std::string&>,
                "Non-const overload");
  static_assert(
      std::is_same_v<decltype(std::as_const(pv)->at(1)), const std::string&>,
      "Const overload");

  // Invokes the const overload and prints "init"
  std::cout << std::as_const(pv)->at(1) << "\n";
  pv->at(1) = "modified"; // Invokes the non-const overload

  // Invokes the const overload and prints "modified"
  std::cout << std::as_const(pv)->at(1) << "\n";
}
```

## See Also

- [concept `proxiable_target`](proxiable_target.md)
- [alias template `skills::as_view`](skills_as_view.md)
