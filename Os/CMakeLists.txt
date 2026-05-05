# pipicube_os: bare-metal HAL glue + GCC atomic runtime for Cortex-M0+.
# F' Os interface implementations are chosen via CHOOSES_IMPLEMENTATIONS in the
# deployment; the Baremetal/*.cpp files use the v3.x Os interface and are not
# compiled to avoid symbol conflicts with the selected F' stubs.
#
# atomic.c provides __atomic_* helpers using PRIMASK interrupt masking because
# the arm-none-eabi sysroot search path excludes the GCC multilib library
# directory where libatomic.a lives, so -latomic fails at link time.
add_library(pipicube_os STATIC atomic.c)
target_link_libraries(pipicube_os PUBLIC pico_stdlib hardware_sync)
