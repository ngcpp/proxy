test_srcs = files(
  'proxy_creation_tests.cpp',
  'proxy_detail_tests.cpp',
  'proxy_dispatch_tests.cpp',
  'proxy_format_tests.cpp',
  'proxy_integration_tests.cpp',
  'proxy_invocation_tests.cpp',
  'proxy_lifetime_tests.cpp',
  'proxy_reflection_tests.cpp',
  'proxy_regression_tests.cpp',
  'proxy_rtti_tests.cpp',
  'proxy_traits_tests.cpp',
  'proxy_view_tests.cpp',
)

test_deps = [msft_proxy4_dep, gtest_dep]

if fmt_dep.found()
  test_srcs += files(
    'proxy_fmt_format_no_wchar_tests.cpp',
    'proxy_fmt_format_tests.cpp',
  )
  test_deps += fmt_dep
endif

test(
  'ProxyTests',
  executable(
    'msft_proxy_tests',
    test_srcs,
    implicit_include_directories: false,
    dependencies: test_deps,
    override_options: {
      'werror': true,
    },
    build_by_default: false,
  ),
  protocol: 'gtest',
)
