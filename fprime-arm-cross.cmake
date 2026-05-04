# Cross-compilation toolchain for ARM Cortex-M0+ (RP2040 / Raspberry Pi Pico)
# Must be included BEFORE project() in CMakeLists.txt.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi)

find_program(CMAKE_C_COMPILER    NAMES ${TOOLCHAIN_PREFIX}-gcc    REQUIRED)
find_program(CMAKE_CXX_COMPILER  NAMES ${TOOLCHAIN_PREFIX}-g++    REQUIRED)
find_program(CMAKE_ASM_COMPILER  NAMES ${TOOLCHAIN_PREFIX}-gcc    REQUIRED)
find_program(CMAKE_AR            NAMES ${TOOLCHAIN_PREFIX}-ar     REQUIRED)
find_program(CMAKE_RANLIB        NAMES ${TOOLCHAIN_PREFIX}-ranlib REQUIRED)
find_program(CMAKE_OBJCOPY       NAMES ${TOOLCHAIN_PREFIX}-objcopy)
find_program(CMAKE_SIZE          NAMES ${TOOLCHAIN_PREFIX}-size)

set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS_INIT    "${CPU_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_INIT  "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${CPU_FLAGS} --specs=nosys.specs -Wl,--gc-sections -Wl,-Map=pipicube.map")

# Sysroot search mode
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
