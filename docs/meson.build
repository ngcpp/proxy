extract_example = find_program('../tools/extract_example_code.py')
gen_doc_examples = find_program('../tools/gen_doc_examples.py')

# Generate the example list at configure time (txt format: "<md>" or "<md>:<var1>,...").
_gen = run_command(gen_doc_examples, '--format', 'txt', check: true)

examples = []

foreach line : _gen.stdout().split('\n')
  if line != ''
    parts = line.split(':')
    md = parts[0]
    dep_vars = parts.length() > 1 ? parts[1].split(',') : []
    extra_deps = []
    foreach dep_var : dep_vars
      extra_deps += [get_variable(dep_var)]
    endforeach

    suffix = md.split('.')[0].replace('/', '_')
    _ex = executable(
      'example_' + suffix,
      custom_target(
        command: [extract_example, '@INPUT@', '@OUTPUT@'],
        input: md,
        output: suffix + '.cpp',
      ),
      extra_files: md,
      implicit_include_directories: false,
      dependencies: [msft_proxy4_dep] + extra_deps,
      build_by_default: false,
    )
    examples += _ex
    test(suffix, _ex, suite: 'ProxyExamples')
  endif
endforeach

alias_target('examples', examples)
