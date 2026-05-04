####
# cmake/platform/RP2040.cmake
#
# F Prime platform file for Raspberry Pi Pico (RP2040).
# Bare-metal target: no POSIX, no OS threads.
# Cooperative scheduling is implemented in Os/Baremetal/.
####

# No POSIX — RP2040 is a bare-metal target
set(FPRIME_USE_POSIX OFF)

# Signal to F' that we use the baremetal cooperative scheduler,
# suppressing the mandatory FIND_PACKAGE(Threads) check.
set(FPRIME_USE_BAREMETAL_SCHEDULER ON)

# PlatformTypes.hpp: the Linux/ variant uses only <cstdint> with no POSIX
# dependency, making it safe for bare-metal use.
include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Fw/Types/Linux")

# OS type definition consumed by F' framework source files
add_definitions(-DTGT_OS_TYPE_BAREMETAL)

# No sockets on bare-metal
set(FPRIME_HAS_SOCKETS OFF)
