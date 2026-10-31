# Named requirements: *ProBasicMetadata*

> Since: 5.0.0

A type `M` meets the *ProBasicMetadata* requirements if `std::is_nothrow_copy_constructible_v<M>` and `std::is_trivially_destructible_v<M>` are `true`.

## Notes

Trivial destructibility allows the library to reuse the storage of metadata without destroying it.

## See Also

- [*ProBasicReflection* requirements](ProBasicReflection.md)
- [*ProMetadata* requirements](ProMetadata.md)
- [*ProMetadataPolicy* requirements](ProMetadataPolicy.md)
