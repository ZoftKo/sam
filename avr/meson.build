lcd = library('lcd', 'lcd.c', include_directories: inc)
usart = library('usart', 'usart.c')

interactive_tx = executable(
  'interactive_tx',
  'interactive_tx.c',
  include_directories: inc,
  name_suffix: 'elf',
  link_with: [tx, lcd, bits, usart]
)
display_rx = executable(
  'display_rx',
  'display_rx.c',
  include_directories: inc,
  name_suffix: 'elf',
  link_with: [rx, lcd, bits, usart]
)

objcopy = find_program('avr-objcopy')
convert_hex_cmd = [objcopy, '-j', '.text', '-j', '.data', '-O', 'ihex', '@INPUT@',  '@OUTPUT@']

interactive_tx_hex = custom_target(
  'interactive_tx_hex',
  input: interactive_tx,
  output: 'interactive_tx.hex',
  command: convert_hex_cmd,
  build_by_default: true
)
display_rx_hex = custom_target(
  'display_rx_hex',
  input: display_rx,
  output: 'display_rx.hex',
  command: convert_hex_cmd,
  build_by_default: true
)
