test(
  'ProxyFreestandingTests',
  executable(
    'msft_proxy_freestanding_tests',
    files('proxy_freestanding_tests.cpp'),
    implicit_include_directories: false,
    dependencies: msft_proxy4_dep,
    override_options: {
      'werror': true,
    },
    build_by_default: false,
  ),
)
