# `basic_facade_builder::add_facade`

```cpp
template <facade F, bool Unused = false>
using add_facade = basic_facade_builder</* see below */>;
```

The alias template `add_facade` of `basic_facade_builder<Ss, Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` adds a [facade](../facade.md) type into the template parameters. Specifically, it

- adds `F` into `Ss`, and
- sets `MaxSize` to `std::min(MaxSize, F::max_size)`, and
- sets `MaxAlign` to `std::min(MaxAlign, F::max_align)`, and
- sets `Copyability` to `std::max(Copyability, F::copyability)`, and
- sets `Relocatability` to `std::max(Relocatability, F::relocatability)`, and
- sets `Destructibility` to `std::max(Destructibility, F::destructibility)`.

*Since 5.0.0*: `F` is added to `Ss`, rather than having its conventions and reflections merged into `Cs` and `Rs`. `Unused` has no effect.

## Notes

The conventions and reflections of `F` are not copied into `Cs` or `Rs`. They are reached through the super. Adding the same facade more than once, or redeclaring a convention that a super already provides, is well-defined and does not have side effects on [`build`](build.md) at either compile-time or runtime.

A convention whose overload is a specialization of [`facade_aware_overload_t`](../facade_aware_overload_t.md) is the exception: its overload depends on the facade it is built into, so it is also checked and made available against the built facade, not only against the super that declares it. [`proxiable`](../proxiable.md) therefore requires the pointer type to satisfy both substitutions, and a failure of either is diagnosed. This is what lets a skill such as [`as_view`](../skills_as_view.md) declared on a super yield a [`proxy_view`](../proxy_view.md) of the built facade as well as of the super.

The metadata of the built facade embeds the metadata of each super, so that converting to a `proxy<F>` needs no indirect call to translate the metadata. The contained value is still copied or relocated as it would be by a copy or a move of a `proxy` of the built facade, which involves an indirect call unless the corresponding [`constraint_level`](../constraint_level.md) is `trivial`. Two consequences of embedding are worth noting. When a super is reachable through more than one other super (a diamond), its metadata is embedded once per path. When the built facade strengthens a [`constraint_level`](../constraint_level.md) that `F` also declares (for example from `nontrivial` to `nothrow`), both levels are represented. Either case makes the metadata larger than the sum of the distinct conventions, and nesting diamonds compounds the effect. Metadata of that size is held out of line and shared by every `proxy` of the facade, so the cost is in static data rather than in `sizeof(proxy<F>)`.

A [`proxy`](../proxy/README.md) of the built facade converts to a `proxy<F>`, subject to the copyability and relocatability of `F`. It also converts to a [`proxy_view`](../proxy_view.md)`<F>` when [`as_view`](../skills_as_view.md) is in effect, and to a [`weak_proxy`](../weak_proxy.md)`<F>` when [`as_weak`](../skills_as_weak.md) is.

## Example

```cpp
#include <iostream>
#include <unordered_map>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemAt, at);
PRO_DEF_MEM_DISPATCH(MemEmplace, emplace);

struct Copyable : pro::facade_builder                               //
                  ::support_copy<pro::constraint_level::nontrivial> //
                  ::build {};

struct BasicContainer
    : pro::facade_builder                                     //
      ::add_convention<MemSize, std::size_t() const noexcept> //
      ::build {};

struct StringDictionary
    : pro::facade_builder                                         //
      ::add_facade<BasicContainer>                                //
      ::add_facade<Copyable>                                      //
      ::add_convention<MemAt, std::string(std::size_t key) const> //
      ::build {};

struct MutableStringDictionary
    : pro::facade_builder                                                    //
      ::add_facade<StringDictionary>                                         //
      ::add_convention<MemEmplace, void(std::size_t key, std::string value)> //
      ::build {};

int main() {
  pro::proxy<MutableStringDictionary> p1 =
      pro::make_proxy<MutableStringDictionary,
                      std::unordered_map<std::size_t, std::string>>();
  std::cout << p1->size() << "\n"; // Prints "0"
  try {
    std::cout << p1->at(123) << "\n"; // No output because the expression throws
  } catch (const std::out_of_range& e) {
    std::cerr << e.what() << "\n"; // Prints error message
  }
  p1->emplace(123, "lalala");
  auto p2 = p1; // Performs a deep copy
  p2->emplace(456, "trivial");

  // Converts to a proxy of the super from an rvalue reference
  pro::proxy<StringDictionary> p3 = std::move(p2);
  std::cout << p1->size() << "\n";  // Prints "1"
  std::cout << p1->at(123) << "\n"; // Prints "lalala"

  // Prints "false" because it is moved
  std::cout << std::boolalpha << p2.has_value() << "\n";
  std::cout << p3->size() << "\n";  // Prints "2"
  std::cout << p3->at(123) << "\n"; // Prints "lalala"
  std::cout << p3->at(456) << "\n"; // Prints "trivial"
}
```

## See Also

- [`build`](build.md)
