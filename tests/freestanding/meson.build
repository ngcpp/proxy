test(
  'ProxyFreestandingTests',
  executable(
    'ngcpp_proxy_freestanding_tests',
    files('proxy_freestanding_tests.cpp'),
    implicit_include_directories: false,
    dependencies: ngcpp_proxy5_dep,
    override_options: {
      'werror': true,
    },
    build_by_default: false,
  ),
)
