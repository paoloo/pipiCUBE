####
# cmake/platform/RP2040.cmake
#
# F Prime platform file for Raspberry Pi Pico (RP2040).
# Bare-metal target: no POSIX, no OS threads.
####

# No POSIX — RP2040 is a bare-metal target
set(FPRIME_USE_POSIX OFF)

# Suppress the mandatory FIND_PACKAGE(Threads) check
set(FPRIME_USE_BAREMETAL_SCHEDULER ON)

# No sockets on bare-metal
set(FPRIME_HAS_SOCKETS OFF)

# Register RP2040 platform config: provides PlatformTypes.fpp (FPP type aliases)
# and PlatformTypes.h (C header for PlatformPointerCastType), and selects the
# F' stub Os implementations required by a bare-metal deployment.
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RP2040/Platform/")
