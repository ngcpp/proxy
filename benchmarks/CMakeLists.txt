project(ngcpp_proxy_benchmarks)

FetchContent_Declare(
  benchmark
  URL ${PROXY_BENCHMARK_URL}
  URL_HASH SHA256=${PROXY_BENCHMARK_SHA256}
)
set(
  BENCHMARK_ENABLE_TESTING
  OFF
  CACHE BOOL
  "Disable tests for google benchmark"
)
set(
  BENCHMARK_ENABLE_GTEST_TESTS
  OFF
  CACHE BOOL
  "Disable google benchmark unit tests"
)
FetchContent_MakeAvailable(benchmark)

add_executable(
  ngcpp_proxy_benchmarks
  proxy_creation_benchmark.cpp
  proxy_operation_benchmark_context.cpp
  proxy_operation_benchmark.cpp
)
target_include_directories(ngcpp_proxy_benchmarks PRIVATE .)
target_link_libraries(
  ngcpp_proxy_benchmarks
  PRIVATE ngcpp_proxy5::proxy benchmark::benchmark benchmark::benchmark_main
)
target_compile_options(
  ngcpp_proxy_benchmarks
  PRIVATE ${PROXY_BUILD_FLAGS} $<$<BOOL:${MSVC}>:/bigobj>
)
