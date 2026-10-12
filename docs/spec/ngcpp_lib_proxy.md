# Feature test macro `__ngcpp_lib_proxy`

> Header: `proxy_macros.h` and `proxy.h`

```cpp
#define __ngcpp_lib_proxy /* see below */
```

Starting with 3.0.0, Proxy ships a feature-test macro that encodes the library version. Releases before Proxy 5 defined this macro as `__msft_lib_proxy`. Starting with Proxy 5, it is named `__ngcpp_lib_proxy`, and the major-qualified form `__ngcpp_lib_proxy<major>` (for example, `__ngcpp_lib_proxy5`) is also available when headers from different major versions can appear in the same translation unit.

| Version | Feature-test macro   | Value     |
| ------- | -------------------- | --------- |
| 5.0.0   | `__ngcpp_lib_proxy`  | `202606L` |
| 4.1.0   | `__msft_lib_proxy`   | `202606L` |
| 4.0.2   | `__msft_lib_proxy`   | `202603L` |
| 4.0.1   | `__msft_lib_proxy`   | `202510L` |
| 4.0.0   | `__msft_lib_proxy`   | `202508L` |
| 3.4.0   | `__msft_lib_proxy`   | `202505L` |
| 3.3.0   | `__msft_lib_proxy`   | `202503L` |
| 3.2.1   | `__msft_lib_proxy`   | `202502L` |
| 3.2.0   | `__msft_lib_proxy`   | `202501L` |
| 3.1.0   | `__msft_lib_proxy`   | `202410L` |
| 3.0.0   | `__msft_lib_proxy`   | `202408L` |

`__msft_lib_proxy` and its major-qualified form `__msft_lib_proxy5` are retained as deprecated aliases for backward compatibility. Supporting compilers issue a deprecation warning when either alias is used.

## Example

```cpp
#include <cstdio>

#include <proxy/proxy.h>

int main() {
#if __ngcpp_lib_proxy >= 202606L
  puts("Compiled with library Proxy 5.0.0 or above.");
#else
  puts("Compiled with an unsupported version of library Proxy.");
#endif
}
```
