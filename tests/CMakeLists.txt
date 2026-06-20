project(msft_proxy_tests)

FetchContent_Declare(
  googletest
  URL ${PROXY_GOOGLETEST_URL}
  URL_HASH SHA256=${PROXY_GOOGLETEST_SHA256}
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding the parent project's compiler/linker settings
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Disable GMock
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(
  msft_proxy_tests
  proxy_creation_tests.cpp
  proxy_detail_tests.cpp
  proxy_dispatch_tests.cpp
  proxy_fmt_format_no_wchar_tests.cpp
  proxy_fmt_format_tests.cpp
  proxy_format_tests.cpp
  proxy_integration_tests.cpp
  proxy_invocation_tests.cpp
  proxy_lifetime_tests.cpp
  proxy_reflection_tests.cpp
  proxy_regression_tests.cpp
  proxy_rtti_tests.cpp
  proxy_traits_tests.cpp
  proxy_view_tests.cpp
)
target_include_directories(msft_proxy_tests PRIVATE .)
target_link_libraries(
  msft_proxy_tests
  PRIVATE msft_proxy4::proxy gtest_main fmt::fmt
)
target_compile_options(msft_proxy_tests PRIVATE ${PROXY_STRICT_WARNING_FLAGS})
gtest_discover_tests(msft_proxy_tests)

add_subdirectory(modules)
