project(
  'sam',
  ['c', 'cpp'],
  version: '0.1'
)

inc = include_directories('include')
subdir('src')

if host_machine.cpu_family() == 'avr'
	subdir('avr')
endif

if not meson.is_cross_build()
	subdir('tests')
endif
