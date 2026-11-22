# `proxy::swap`

```cpp
// (1)
void swap(proxy& rhs)
    noexcept(F::relocatability >= lifetime_requirement::nothrow ||
        F::copyability == lifetime_requirement::trivial)
    requires(F::relocatability == lifetime_requirement::trivial ||
        F::copyability == lifetime_requirement::trivial ||
        (F::relocatability >= lifetime_requirement::supported &&
            F::destructibility >= lifetime_requirement::supported));

// (2) (since 5.0.0)
void swap(proxy& rhs)
    noexcept(F::copyability >= lifetime_requirement::nothrow &&
        F::destructibility >= lifetime_requirement::nothrow)
    requires(F::relocatability == lifetime_requirement::none &&
        (F::copyability == lifetime_requirement::supported ||
            F::copyability == lifetime_requirement::nothrow) &&
        F::destructibility >= lifetime_requirement::supported);
```

Exchanges the contained values of `*this` and `rhs`.

- `(1)` Exchanges the values by relocation, or by exchanging the underlying storage when `F::relocatability == lifetime_requirement::trivial` or `F::copyability == lifetime_requirement::trivial` is `true`. If the relocation throws when `F::relocatability == lifetime_requirement::supported`, both operands can be left without a value.
- `(2)` Exchanges the values by copying, for a facade that forbids relocation. If a copy throws when `F::copyability == lifetime_requirement::supported`, one of the two operands can be left without a value.
