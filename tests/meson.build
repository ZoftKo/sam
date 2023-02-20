cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.append_compile_args('cpp', '-w')
opt_var.append_link_args('-w')

gtest_proj = cmake.subproject('gtest', options: opt_var)
gtest = declare_dependency(
  dependencies: [
	dependency('threads'),
	gtest_proj.dependency('gtest'),
    gtest_proj.dependency('gtest_main')
  ]
)

test_tx = executable(
  'test_tx',
  'test_tx.cc',
  include_directories: inc,
  dependencies: [gtest],
  link_with: [tx]
)
test_rx = executable(
  'test_rx',
  'test_rx.cc',
  include_directories: inc,
  dependencies: [gtest],
  link_with: [tx, rx]
)
test_bits = executable(
  'test_bits',
  'test_bits.cc',
  include_directories: inc,
  dependencies: [gtest],
  link_with: [bits]
)

test('gtest_tx', test_tx)
test('gtest_rx', test_rx)
test('gtest_bits', test_bits)
