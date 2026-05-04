cmake_minimum_required(VERSION 3.26)

# Toolchain must be set before project()
include("${CMAKE_CURRENT_LIST_DIR}/fprime-arm-cross.cmake")

project(pipicube C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

# ── Pico SDK ─────────────────────────────────────────────────────────────────
if(NOT DEFINED ENV{PICO_SDK_PATH})
  message(FATAL_ERROR "Set PICO_SDK_PATH to the pico-sdk root directory")
endif()
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
include("${PICO_SDK_PATH}/pico_sdk_init.cmake")
pico_sdk_init()

# ── F Prime framework ─────────────────────────────────────────────────────────
if(NOT DEFINED FPRIME_FRAMEWORK_PATH)
  set(FPRIME_FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/fprime")
endif()
if(NOT EXISTS "${FPRIME_FRAMEWORK_PATH}/cmake/FPrime.cmake")
  message(FATAL_ERROR
    "F Prime not found at ${FPRIME_FRAMEWORK_PATH}. "
    "Run: git submodule update --init --recursive")
endif()

# Tell F' where to look for custom platform files (checked before the framework
# directory, so cmake/platform/RP2040.cmake in this repo is found first).
set(FPRIME_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}")

# Select the RP2040 bare-metal platform file (cmake/platform/RP2040.cmake).
# Must be set before find_package — platform detection runs at include time.
set(FPRIME_PLATFORM "RP2040")

# Load F' framework modules (Fw_*, Svc_*, etc.) alongside project modules.
set(FPRIME_INCLUDE_FRAMEWORK_CODE ON)

find_package(FPrime REQUIRED PATHS "${FPRIME_FRAMEWORK_PATH}")

# ── Project-wide compile options (space-grade) ───────────────────────────────
include("${CMAKE_CURRENT_LIST_DIR}/cmake/rp2040.cmake")

# Project config headers (FpConfig.h, AcConstants.fpp)
include_directories("${CMAKE_CURRENT_LIST_DIR}/config")

# ── Subdirectories ────────────────────────────────────────────────────────────
# hal and Os are plain CMake libraries — not F' modules, use add_subdirectory
add_subdirectory(hal)
add_subdirectory(Os)
# Components and Top are F' modules — must use add_fprime_subdirectory so that
# F' sets FPRIME_CURRENT_MODULE before entering each CMakeLists.txt
add_fprime_subdirectory(Components)
add_fprime_subdirectory(Top)

register_fprime_deployment(pipicube
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/Top/main.cpp"
  DEPENDS
    pipicube_Top
    pipicube_hal
    pipicube_os
    pico_stdlib
    hardware_timer
)

pico_add_extra_outputs(pipicube)
pico_enable_stdio_usb(pipicube 1)
pico_enable_stdio_uart(pipicube 0)
