# RP2040-specific compile options enforcing space-grade coding standards.
# Included by the root CMakeLists.txt after project() and pico_sdk_init().

add_compile_options(
  -Wall
  -Wextra
  -Wshadow
  -Wcast-align
  -Wvla                      # forbid variable-length arrays (SEI CERT ARR32-C)
  -Wstack-usage=2048          # warn if a function exceeds the smallest task stack
  -Wdouble-promotion
  -Wno-unused-parameter
  -fno-common
  -ffreestanding
)

# Hard-fault on stack overflows via GCC stack protector (canary)
add_compile_options(-fstack-protector-strong)

add_compile_definitions(
  F_PRIME_BAREMETAL=1
  PICO_HEAP_SIZE=0
)
