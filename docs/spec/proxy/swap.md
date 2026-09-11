# `proxy::swap`

```cpp
// (1)
void swap(proxy& rhs)
    noexcept(F::relocatability >= constraint_level::nothrow ||
        F::copyability == constraint_level::trivial)
    requires(F::relocatability >= constraint_level::nontrivial ||
        F::copyability == constraint_level::trivial);

// (2) (since 5.0.0)
void swap(proxy& rhs)
    noexcept(F::copyability >= constraint_level::nothrow &&
        F::destructibility >= constraint_level::nothrow)
    requires(F::relocatability == constraint_level::none &&
        (F::copyability == constraint_level::nontrivial ||
            F::copyability == constraint_level::nothrow) &&
        F::destructibility >= constraint_level::nontrivial);
```

Exchanges the contained values of `*this` and `rhs`.

- `(1)` Exchanges the values by relocation, or by exchanging the underlying storage when `F::relocatability == constraint_level::trivial` or `F::copyability == constraint_level::trivial` is `true`. If the relocation throws when `F::relocatability == constraint_level::nontrivial`, both operands can be left without a value.
- `(2)` Exchanges the values by copying, for a facade that forbids relocation. If a copy throws when `F::copyability == constraint_level::nontrivial`, one of the two operands can be left without a value.
