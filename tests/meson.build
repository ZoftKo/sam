cmake = import('cmake')
gtest_proj = cmake.subproject('gtest')
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
test('gtest_tx', test_tx)
