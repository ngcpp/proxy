# Enum class `lifetime_requirement`<br />Type alias `constraint_level`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v5`

```cpp
enum class lifetime_requirement : /* unspecified */ {
  none, supported, nothrow, trivial,
  nontrivial = supported  // deprecated
};

using constraint_level = lifetime_requirement;  // deprecated
```

`lifetime_requirement` defines the requirements on a *lifetime operation* of a type, that is, its copy construction, relocation, or destruction. For a given lifetime operation `O`:

- `none`: No restrictions on `O`.
- `supported`: `O` shall be supported.
- `nothrow`: `O` shall be supported and shall not throw.
- `trivial`: `O` shall be supported and shall be trivial.

The specific semantics of each value depends on its context.

*Since 5.0.0*: `lifetime_requirement` replaces `constraint_level`, and `supported` replaces `nontrivial`. `constraint_level` and `nontrivial` are retained as deprecated aliases for backward compatibility and may be removed in a future release.

## See Also

- [concept `facade`](facade.md)
