project(
  'sam',
  ['c', 'cpp'],
  version: '0.1',
  default_options: [
    'warning_level=3',
    'werror=true',
    'c_std=c17',
    'cpp_std=c++20'
    ]
)

if host_machine.cpu_family() == 'avr'
  pgsize_param = '--param=min-pagesize=0'
  compiler = meson.get_compiler('c')
  if compiler.has_argument(pgsize_param)
    add_global_arguments(pgsize_param, language: 'c')
  endif
endif

inc = include_directories('include')
subdir('src')

if host_machine.cpu_family() == 'avr'
  subdir('avr')
endif

if not meson.is_cross_build()
  subdir('tests')
endif
