# Class template `proxy_operation`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`  
> Since: 5.0.0

```cpp
template <class D, class S>
struct proxy_operation {};
```

Class template `proxy_operation` is a *descriptor* of an operation performed on a [`proxy`](proxy/README.md) with a [dispatch](ProxyDispatch.md) type `D` and a [signature](ProxySignature.md) type `S`. Descriptors are the template arguments following `Self` of the member template `accessor` of an access type (see [*ProxyAccess* requirements](ProxyAccess.md)), telling the access type which operations and reflections its accessor provides accessibility for.

For an operation type `O` of a [facade](facade.md) type `F` or of any super of `F`, the descriptor of `O` in [`proxy<F, MP>`](proxy/README.md) or [`proxy_indirect_accessor<F, MP>`](proxy_indirect_accessor/README.md) is `proxy_operation<typename O::dispatch_type, `[`substituted-signature`](ProxySignature.md)`<typename O::signature_type, F, MP>>`. Within an accessor, the described operation is performed by calling [`invoke<D, S>`](proxy/friend_invoke.md) with the `Self` object, qualified with the *cv ref* qualifiers of `S`, and the arguments of `S`. The access type of an operation added by [`basic_facade_builder`](basic_facade_builder/add_convention.md) is the default access type of its dispatch type.

## Example

```cpp
#include <iostream>
#include <tuple>

#include <proxy/proxy.h>

struct AreaAccess {
  template <class Self, class... Ds>
  struct accessor {
    accessor() = delete;
  };
  template <class Self, class D>
  struct accessor<Self, pro::proxy_operation<D, double() const>> {
    double Area() const {
      return invoke<D, double() const>(static_cast<const Self&>(*this));
    }
  };
};

struct AreaDispatch {
  template <class T>
  double operator()(const T& self) const {
    return self.Width() * self.Height();
  }
};

struct AreaOperation {
  static constexpr bool is_direct = false;
  using dispatch_type = AreaDispatch;
  using signature_type = double() const;
  using access_type = AreaAccess;
};

struct Shape {
  using super_types = std::tuple<>;
  using operation_types = std::tuple<AreaOperation>;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::lifetime_requirement::none;
  static constexpr auto relocatability = pro::lifetime_requirement::trivial;
  static constexpr auto destructibility = pro::lifetime_requirement::nothrow;
};

class Rectangle {
public:
  Rectangle(double width, double height) : width_(width), height_(height) {}
  double Width() const { return width_; }
  double Height() const { return height_; }

private:
  double width_;
  double height_;
};

int main() {
  pro::proxy<Shape> p = pro::make_proxy<Shape, Rectangle>(3, 5);
  std::cout << p->Area() << "\n"; // Prints "15"
}
```

## See Also

- [class template `proxy_reflection`](proxy_reflection.md)
- [named requirements *ProxyAccess*](ProxyAccess.md)
- [named requirements *ProxyBasicOperation*](ProxyBasicOperation.md)
