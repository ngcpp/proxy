# Class `compact_metadata`<br />Class `inline_metadata`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
struct compact_metadata;
struct inline_metadata;
```

`compact_metadata` and `inline_metadata` are the metadata policies provided by the library. Both meet the [*ProMetadataPolicy* requirements](ProMetadataPolicy.md) and use the same invoker type, a pointer to the generated function. They differ only in how a [`proxy`](proxy/README.md) keeps the metadata deduced from the contained type.

| Name               | Metadata storage                                             |
| ------------------ | ------------------------------------------------------------ |
| `compact_metadata` | The metadata is kept in the `proxy` object when it is no larger than a pointer, and otherwise the `proxy` keeps a pointer to a static metadata object of the contained type. |
| `inline_metadata`  | The metadata is always kept in the `proxy` object.            |

`compact_metadata` is the default metadata policy of `proxy` and of every function template that creates a `proxy`.

A `proxy` converts only to a `proxy` with the same metadata policy, so the policy is chosen where the `proxy` type is named and is preserved by every conversion to a super, by [`skills::as_view`](skills_as_view.md), by [`skills::as_weak`](skills_as_weak.md) and by [`weak_proxy::lock`](weak_proxy.md).

## Notes

The two policies differ only when the metadata is larger than a pointer. Otherwise `compact_metadata` also keeps the metadata in the `proxy`, and both policies produce the same layout.

When the metadata is larger than a pointer, `compact_metadata` keeps a pointer to a static metadata object, so an invocation loads that pointer before it can load the invoker. `inline_metadata` removes that dependent load but keeps the whole metadata in the `proxy`, which makes it larger. The saved load tends to reduce invocation latency when the invoked proxies stay in cache, for example in a hot loop. The larger size tends to cost more when they do not, because fewer proxies fit in cache and iterating over them moves more memory. The performance benefit depends on the metadata size, the access pattern and the cache behavior.

The static metadata object kept by `compact_metadata` is never destroyed, so a `proxy` that refers to it remains usable while other objects with static storage duration are destroyed. The object is constant-initialized when the metadata can be constructed in a constant expression. Otherwise it is dynamically initialized, and its initialization is unordered with respect to the dynamic initialization of other variables. In that case, using such a `proxy` for anything other than its construction during the dynamic initialization of a variable with static storage duration results in undefined behavior.

A `proxy` contains storage for the pointer and for the metadata representation chosen by its policy. Its total size also depends on alignment and padding.

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

  // The metadata of Stringable is larger than a pointer, so
  // keeping it inline makes the proxy larger
  std::cout << std::boolalpha << (sizeof(p2) > sizeof(p1))
            << "\n"; // Prints "true"
}
```

## See Also

- [class template `proxy`](proxy/README.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
