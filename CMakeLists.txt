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
include("${FPRIME_FRAMEWORK_PATH}/cmake/FPrime.cmake")

# ── Project-wide compile options (space-grade) ───────────────────────────────
include("${CMAKE_CURRENT_LIST_DIR}/cmake/rp2040.cmake")

# Project config headers (FpConfig.h, AcConstants.fpp)
include_directories("${CMAKE_CURRENT_LIST_DIR}/config")

# ── Subdirectories ────────────────────────────────────────────────────────────
add_subdirectory(hal)
add_subdirectory(Os)
add_subdirectory(Components)
add_subdirectory(Top)
