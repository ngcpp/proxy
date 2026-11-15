# Named requirements: *ProxyBasicMetadata*

> Since: 5.0.0

A type `M` meets the *ProxyBasicMetadata* requirements if `std::is_nothrow_copy_constructible_v<M>` and `std::is_trivially_destructible_v<M>` are `true`.

## Notes

Trivial destructibility allows the library to reuse the storage of metadata without destroying it.

## See Also

- [*ProxyBasicReflection* requirements](ProxyBasicReflection.md)
- [*ProxyMetadata* requirements](ProxyMetadata.md)
- [*ProxyMetadataPolicy* requirements](ProxyMetadataPolicy.md)
