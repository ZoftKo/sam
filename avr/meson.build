testtx = executable(
    'testtx', 'testtx.c', include_directories: inc,  name_suffix: 'elf', link_with: tx
)

objcopy = find_program('avr-objcopy')
testtx_hex = custom_target(
    'testtx_hex',
    input: testtx,
    output: 'testtx.hex',
    command: [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex', '@INPUT@',  '@OUTPUT@'],
    build_by_default: true
)
