# Class template `operator_access::accessor`

```cpp
// (1)
template <class Self, class... Ds>
struct accessor {
  accessor() = delete;
};
```

`(1)` The default implementation of `accessor` is not constructible.

For different `Sign` and `Rhs`, `operator_access<Sign, Rhs>::accessor` has different specializations. `sop` denotes the sign of operator of each specialization. Each type in `Ds` is a *descriptor* (see [*ProAccess* requirements](../ProAccess.md)), and an operation is described by [`proxy_operation`](../proxy_operation.md)`<D, O>`, where `D` is a [dispatch](../ProDispatch.md) type and `O` is an [overload](../ProOverload.md) type.

## Left-Hand-Side Operand Specializations

```cpp
// (2)
template <class Self, class... Ds>
    requires(sizeof...(Ds) > 1u && (std::is_constructible_v<accessor<Self, Ds>> && ...))
struct accessor<Self, Ds...> : accessor<Self, Ds>... {
  using accessor<Self, Ds>::operator sop...;
};
```

`(2)` When `sizeof...(Ds)` is greater than `1`, and `accessor<Self, Ds>...` are default-constructible types, inherits all `accessor<Self, Ds>...` types and `using` their `operator sop`.

When `Rhs` is `false`, the other specializations are defined as follows, where `sizeof...(Ds)` is `1` and the only type in `Ds` is `proxy_operation<D, O>`, with `O` qualified with `cv ref noex`:

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`, `"()"`, `"[]"`,

```cpp
// (3)
template <class Self, class D, class R, class... Args>
struct accessor<Self, proxy_operation<D, R(Args...) cv ref noex>> {
  R operator sop (Args... args) cv ref noex;
};
```

`(3)` Provides an `operator sop(Args...)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Args...)` is equivalent to `return invoke<D, R(Args...) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this), std::forward<Args>(args)...)`.

### `"!"` and `"~"`

When `Sign` is either `"!"` or `"~"`,

```cpp
// (4)
template <class Self, class D, class R>
struct accessor<Self, proxy_operation<D, R() cv ref noex>> {
  R operator sop () cv ref noex;
};
```

`(4)` Provides an `operator sop()` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop()` is equivalent to `return invoke<D, R() cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this))`.

### `"++"` and `"--"`

When `Sign` is either `"++"` or `"--"`,

```cpp
// (5)
template <class Self, class D, class R>
struct accessor<Self, proxy_operation<D, R() cv ref noex>> {
  /* see below */ operator sop () cv ref noex;
};

// (6)
template <class Self, class D, class R>
struct accessor<Self, proxy_operation<D, R(int) cv ref noex>> {
  R operator sop (int) cv ref noex;
};
```

`(5)` Provides a prefix `operator sop()` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop()` calls `invoke<D, R() cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this))` and returns `static_cast<Self cv <ref ? ref : &>>(*this)`.

`(6)` Provides a postfix `operator sop(int)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(int)` is equivalent to `return invoke<D, R(int) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this), 0)`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"%="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (7)
template <class Self, class D, class R, class Arg>
struct accessor<Self, proxy_operation<D, R(Arg) cv ref noex>> {
  /* see below */ operator sop (Arg arg) cv ref noex;
};
```

`(7)` Provides an `operator sop(Arg)` with the same *cv ref noex* specifiers as of the overload type. `accessor::operator sop(Arg)` calls `invoke<D, R(Arg) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(*this), std::forward<Arg>(arg))` and returns `static_cast<Self cv <ref ? ref : &>>(*this)`.

## Right-Hand-Side Operand Specializations

```cpp
// (8)
template <class Self, class... Ds>
    requires(sizeof...(Ds) > 1u && (std::is_constructible_v<accessor<Self, Ds>> && ...))
struct accessor<Self, Ds...> : accessor<Self, Ds>... {};
```

`(8)` When `sizeof...(Ds)` is greater than `1`, and `accessor<Self, Ds>...` are default-constructible types, inherits all `accessor<Self, Ds>...` types.

When `Rhs` is `true`, the other specializations are defined as follows, where `sizeof...(Ds)` is `1` and the only type in `Ds` is `proxy_operation<D, O>`, with `O` qualified with `cv ref noex`:

### Regular SOPs

When `Sign` is one of `"+"`, `"-"`, `"*"`, `"/"`, `"%"`, `"=="`, `"!="`, `">"`, `"<"`, `">="`, `"<="`, `"<=>"`, `"&&"`, `"||"`, `"&"`, `"|"`, `"^"`, `"<<"`, `">>"`, `","`, `"->*"`,

```cpp
// (9)
template <class Self, class D, class R, class Arg>
struct accessor<Self, proxy_operation<D, R(Arg) cv ref noex>> {
  friend R operator sop (Arg arg, Self cv <ref ? ref : &> self) noex;
};
```

`(9)` Provides a `friend operator sop(Arg arg, Self cv <ref ? ref : &> self)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg arg, Self cv <ref ? ref : &> self)` is equivalent to `return invoke<D, R(Arg) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(self), std::forward<Arg>(arg))`.

### Assignment SOPs

When `Sign` is one of `"+="`, `"-="`, `"*="`, `"/="`, `"%="`, `"&="`, `"|="`, `"^="`, `"<<="`, `">>="`,

```cpp
// (10)
template <class Self, class D, class R, class Arg>
struct accessor<Self, proxy_operation<D, R(Arg&) cv ref noex>> {
  friend Arg& operator sop (Arg& arg, Self cv <ref ? ref : &> self) noex;
};
```

`(10)` Provides a `friend operator sop(Arg& arg, Self cv <ref ? ref : &> self)` with the same *noex* specifiers as of the overload type. `accessor::operator sop(Arg& arg, Self cv <ref ? ref : &> self)` calls `invoke<D, R(Arg&) cv ref noex>(static_cast<Self cv <ref ? ref : &>>(self), arg)` and returns `arg`.
