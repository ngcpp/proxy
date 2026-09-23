# Class `compact_metadata`<br />Class `inline_metadata`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
struct compact_metadata;
struct inline_metadata;
```

`compact_metadata` and `inline_metadata` are the metadata policies provided by the library. Both meet the [*ProMetadataPolicy* requirements](ProMetadataPolicy.md) and erase an invocation the same way. They differ only in how a [`proxy`](proxy/README.md) keeps the metadata deduced from the contained type.

| Name               | Metadata storage                                             |
| ------------------ | ------------------------------------------------------------ |
| `compact_metadata` | The metadata is kept in the `proxy` object when it is no larger than a pointer, and otherwise the `proxy` keeps a pointer to a static metadata object of the contained type. |
| `inline_metadata`  | The metadata is always kept in the `proxy` object.            |

`compact_metadata` is the default metadata policy of `proxy` and of every function template that creates a `proxy`.

A `proxy` converts only to a `proxy` with the same metadata policy, so the policy is chosen where the `proxy` type is named and is preserved by every conversion to a super, by [`skills::as_view`](skills_as_view.md), by [`skills::as_weak`](skills_as_weak.md) and by [`weak_proxy::lock`](weak_proxy.md).

## Notes

`inline_metadata` trades size for one fewer indirection on every invocation. A `proxy` with `inline_metadata` is as large as its pointer storage plus its whole metadata, while an invocation reads the invoker directly from the `proxy` instead of following a pointer to a static metadata object. `compact_metadata` keeps `sizeof(proxy<F>)` at the size of the pointer storage plus one pointer, at the cost of that indirection when the metadata is larger than a pointer.

The indirection of `compact_metadata` costs more than one load when the `proxy` is long-lived and invoked from a cold path. The static metadata object lives away from the `proxy`, so reaching it touches a cache line the caller would not otherwise bring in, while inline metadata rides in the lines already fetched for the `proxy` itself. `inline_metadata` is aimed at that case, and at latency-sensitive code in general, rather than at throughput in a hot loop where the static object stays cached.

## Example

```cpp
#include <iostream>
#include <string>

#include <proxy/proxy.h>

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder                                 //
                    ::add_convention<FreeToString, std::string() const> //
                    ::support_copy<pro::constraint_level::nontrivial>   //
                    ::build {};

int main() {
  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(123);
  pro::proxy<Stringable, pro::inline_metadata> p2 =
      pro::make_proxy<Stringable, int, pro::inline_metadata>(123);
  std::cout << ToString(*p1) << "\n"; // Prints "123"
  std::cout << ToString(*p2) << "\n"; // Prints "123"

  // Keeping the metadata inline makes the proxy larger
  std::cout << std::boolalpha << (sizeof(p2) > sizeof(p1))
            << "\n"; // Prints "true"
}
```

## See Also

- [class template `proxy`](proxy/README.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
