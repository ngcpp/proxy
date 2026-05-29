project(
  'msft_proxy4',
  'cpp',
  version: '4.0.2',
  license: 'MIT',
  license_files: 'LICENSE',
  meson_version: '>=1.3',
  default_options: {
    'cpp_std': ['vc++latest', 'c++26', 'c++23', 'c++20'],
    'warning_level': '3',
  },
)

cxx = meson.get_compiler('cpp')
if cxx.get_argument_syntax() == 'msvc'
  add_project_arguments(
    '/utf-8',
    language: 'cpp',
  )
elif cxx.get_id() == 'clang'
  add_project_arguments(
    '-Wno-c++2b-extensions',
    language: 'cpp',
  )
elif cxx.get_id() == 'nvidia_hpc'
  add_project_arguments(
    '-Minform=warn', '-Wall', '-Wextra', '-Wpedantic',
    language: 'cpp',
  )
endif

inc = include_directories('include')

hdrs = files(
  'include/proxy/proxy.h',
  'include/proxy/proxy_fmt.h',
  'include/proxy/proxy_macros.h',
)
hdrs_v4 = files(
  'include/proxy/v4/proxy.h',
  'include/proxy/v4/proxy_fmt.h',
  'include/proxy/v4/proxy_macros.h',
)
hdrs_mod = files('include/proxy/v4/proxy.ixx')

msft_proxy4_dep = declare_dependency(
  sources: hdrs + hdrs_v4,
  include_directories: inc,
)

install_headers(
  hdrs,
  subdir: 'proxy',
)
install_headers(
  hdrs_v4,
  subdir: 'proxy/v4',
)
install_headers(
  hdrs_mod,
  subdir: 'proxy/v4',
)
pkgconfig = import(
  'pkgconfig',
  required: false,
)
if pkgconfig.found()
  pkgconfig.generate(
    name: meson.project_name(),
    description: 'Next Generation Polymorphism in C++',
    url: 'https://ngcpp.github.io/proxy/',
  )
endif

meson.override_dependency(meson.project_name(), msft_proxy4_dep)

tests = get_option('tests').disable_auto_if(meson.is_subproject())
gtest_dep = dependency(
  'gtest_main',
  main: true,
  required: tests,
  default_options: ['warning_level=0'],
)
fmt_dep = dependency(
  'fmt',
  version: '>=6.1.0',
  required: tests,
  disabler: true,
  default_options: ['warning_level=0'],
  include_type: 'system',
)
subdir(
  'tests',
  if_found: gtest_dep,
)

benchmarks = get_option('benchmarks').disable_auto_if(meson.is_subproject())
benchmark_dep = dependency(
  'benchmark_main',
  required: benchmarks,
  default_options: ['warning_level=0'],
)
subdir(
  'benchmarks',
  if_found: benchmark_dep,
)

if tests.allowed() and get_option('doc_tests')
  subdir('docs')
endif
