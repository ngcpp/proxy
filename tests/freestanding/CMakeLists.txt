add_executable(ngcpp_proxy_freestanding_tests proxy_freestanding_tests.cpp)
target_compile_options(
  ngcpp_proxy_freestanding_tests
  PRIVATE ${PROXY_STRICT_WARNING_FLAGS}
)
target_link_libraries(ngcpp_proxy_freestanding_tests PRIVATE ngcpp_proxy5::proxy)
add_test(NAME ProxyFreestandingTests COMMAND ngcpp_proxy_freestanding_tests)

if(PROXY_BUILD_MODULES)
  add_executable(
    ngcpp_proxy_freestanding_module_tests
    proxy_freestanding_module_tests.cpp
  )
  target_compile_options(
    ngcpp_proxy_freestanding_module_tests
    PRIVATE ${PROXY_STRICT_WARNING_FLAGS}
  )
  target_link_libraries(
    ngcpp_proxy_freestanding_module_tests
    PRIVATE ngcpp_proxy5::proxy_module
  )
  add_test(
    NAME ProxyFreestandingModuleSupportTests
    COMMAND ngcpp_proxy_freestanding_module_tests
  )
endif()
