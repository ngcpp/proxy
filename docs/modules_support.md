# Modules Support

> Since: 4.0.0

The "Proxy" library ships with `.ixx` files starting with version **4.0.0**. Compared to traditional headers, modules offer faster compilation speed and isolation against preprocessor macro definitions.

As of 2025-05-11, CMake lacks support for forward compatibility when consuming C++ modules, which causes consumers with newer C++ standard to be unable to use modules with older standard. Until this is implemented by CMake, a CMake target containing the module can be manually declared using the following CMake script:

```cmake
find_package(ngcpp_proxy5 REQUIRED)

if(NOT DEFINED ngcpp_proxy5_INCLUDE_DIR) # (1)
  if(NOT DEFINED ngcpp_proxy5_SOURCE_DIR)
    message(FATAL_ERROR "`ngcpp_proxy5_INCLUDE_DIR` or `ngcpp_proxy5_SOURCE_DIR` must be defined to use this script.")
  endif()
  set(ngcpp_proxy5_INCLUDE_DIR ${ngcpp_proxy5_SOURCE_DIR}/include)
endif()

message(STATUS "Declaring `ngcpp_proxy5::proxy_module` target for include path `${ngcpp_proxy5_INCLUDE_DIR}`")

add_library(ngcpp_proxy5_module)
set_target_properties(
  ngcpp_proxy5_module
  PROPERTIES
    SYSTEM TRUE
    EXCLUDE_FROM_ALL TRUE
)

add_library(ngcpp_proxy5::proxy_module ALIAS ngcpp_proxy5_module)
target_sources(ngcpp_proxy5_module PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${ngcpp_proxy5_INCLUDE_DIR}
  FILES
    ${ngcpp_proxy5_INCLUDE_DIR}/proxy/v5/proxy.ixx
)
target_compile_features(ngcpp_proxy5_module PUBLIC cxx_std_20) # (2)
target_link_libraries(ngcpp_proxy5_module PUBLIC ngcpp_proxy5::proxy)
```

- (1) `ngcpp_proxy5_INCLUDE_DIR` is automatically declared after `find_package(ngcpp_proxy5)`. CPM uses a slightly different convention where `ngcpp_proxy5_SOURCE_DIR` is declared after `CPMAddPackage`.
- (2) The C++ standard version for `ngcpp_proxy5_module` target should be the same or higher than the consumer CMake target. For example if your project is using C++23 mode, this line should be changed to `cxx_std_23` or `cxx_std_26` / newer standards.

It can then be consumed like this:

```cmake
target_link_libraries(main PRIVATE ngcpp_proxy5::proxy_module)
```

## Example

Module definition:

```cpp
// dictionary.cpp
module;

#include <string> // (1)

#include <proxy/proxy_macros.h> // (2)

export module dictionary;

import proxy.v5; // (3)

extern "C++" { // (4)
PRO_DEF_MEM_DISPATCH(MemAt, at);
}

export struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int index) const>
    ::build {};

```

Client:

```cpp
// main.cpp
#include <vector>
#include <iostream>

import proxy.v5;
import dictionary;

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n";  // Prints "world"
}

```

- (1) This is a traditional header rather than a module. It should be declared in global fragment (after `module` and before `export module`).
- (2) This makes all `PRO_DEF_` macros available. This header file contains only some macros and are therefore very fast to compile.
- (3) `import proxy.v5;` makes all public interfaces from `pro::v5` namespace available in the current translation unit.
- (4) As of 2025-05-11, clangd requires the accessor struct to be either `export`-ed, or be declared within an `extern "C++"` block, in order to have auto completion working.
