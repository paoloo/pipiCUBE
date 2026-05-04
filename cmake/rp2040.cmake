# RP2040-specific compile options enforcing space-grade coding standards.
# Included by the root CMakeLists.txt after project() and pico_sdk_init().

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wcast-align
  -Wvla                      # forbid variable-length arrays (SEI CERT ARR32-C)
  -Wstack-usage=1024          # warn if a function exceeds 1 kB of stack
  -Wdouble-promotion
  -Wno-unused-parameter
  -fno-common
  -ffreestanding
)

# Hard-fault on stack overflows via GCC stack protector (canary)
add_compile_options(-fstack-protector-strong)

add_compile_definitions(
  PICO_BOARD=pico
  PICO_RP2040=1
  F_PRIME_BAREMETAL=1
  # Disable heap after init: all allocations must be static
  PICO_HEAP_SIZE=0
)
