add_executable(msft_proxy_freestanding_tests proxy_freestanding_tests.cpp)
target_compile_options(
  msft_proxy_freestanding_tests
  PRIVATE ${PROXY_STRICT_WARNING_FLAGS}
)
target_link_libraries(msft_proxy_freestanding_tests PRIVATE msft_proxy4::proxy)
add_test(NAME ProxyFreestandingTests COMMAND msft_proxy_freestanding_tests)

if(PROXY_BUILD_MODULES)
  add_executable(
    msft_proxy_freestanding_module_tests
    proxy_freestanding_module_tests.cpp
  )
  target_compile_options(
    msft_proxy_freestanding_module_tests
    PRIVATE ${PROXY_STRICT_WARNING_FLAGS}
  )
  target_link_libraries(
    msft_proxy_freestanding_module_tests
    PRIVATE msft_proxy4::proxy_module
  )
  add_test(
    NAME ProxyFreestandingModuleSupportTests
    COMMAND msft_proxy_freestanding_module_tests
  )
endif()
