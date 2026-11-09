# Class template `implicit_conversion_access::accessor`

```cpp
// (1)
template <class Self, class... Ds>
struct accessor {
  accessor() = delete;
};

// (2)
template <class Self, class... Ds>
    requires(sizeof...(Ds) > 1u && (std::is_constructible_v<accessor<Self, Ds>> && ...))
struct accessor<Self, Ds...> : accessor<Self, Ds>... {
  using accessor<Self, Ds>::operator return-type-of<Ds>...;
};

// (3)
template <class Self, class D, class T>
struct accessor<Self, proxy_operation<D, T() cv ref noex>> {
  operator T() cv ref noex;
};
```

`(1)` The default implementation of `accessor` is not constructible.

`(2)` When `sizeof...(Ds)` is greater than `1`, and `accessor<Self, Ds>...` are default-constructible, inherits all `accessor<Self, Ds>...` types and `using` their `operator return-type-of<Ds>`. For a *descriptor* (see [*ProAccess* requirements](../ProAccess.md)) [`proxy_operation`](../proxy_operation.md)`<D, O>`, `return-type-of<proxy_operation<D, O>>` denotes the *return type* of the overload type `O`.

`(3)` When `sizeof...(Ds)` is `1` and the only type in `Ds` is `proxy_operation<D, T() cv ref noex>`, provides an implicit `operator T()` with the same *cv ref noex* specifiers. `accessor::operator T()` is equivalent to `return invoke<D, T() cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this))`.
