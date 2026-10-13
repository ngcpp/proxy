if(NOT DEFINED ngcpp_proxy5_INCLUDE_DIR)
  message(
    FATAL_ERROR
    "`ngcpp_proxy5_INCLUDE_DIR` must be defined to use this script."
  )
endif()

message(
  STATUS
  "Declaring `ngcpp_proxy5::proxy_module` target for include path `${ngcpp_proxy5_INCLUDE_DIR}`"
)

add_library(ngcpp_proxy5_module)
set_target_properties(
  ngcpp_proxy5_module
  PROPERTIES SYSTEM TRUE EXCLUDE_FROM_ALL TRUE
)

add_library(ngcpp_proxy5::proxy_module ALIAS ngcpp_proxy5_module)
target_sources(
  ngcpp_proxy5_module
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS ${ngcpp_proxy5_INCLUDE_DIR}
      FILES ${ngcpp_proxy5_INCLUDE_DIR}/proxy/v5/proxy.ixx
)
target_compile_features(ngcpp_proxy5_module PUBLIC cxx_std_20)
target_compile_options(
  ngcpp_proxy5_module
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-c++2b-extensions>
)
target_link_libraries(ngcpp_proxy5_module PUBLIC ngcpp_proxy5::proxy)
