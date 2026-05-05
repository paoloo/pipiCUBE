####
# cmake/platform/RP2040/Platform/CMakeLists.txt
#
# Base configuration for the RP2040 bare-metal platform.
# Provides FPP platform type aliases and selects F' stub Os implementations.
####
register_fprime_config(
    RP2040PlatformTypes
  AUTOCODER_INPUTS
    "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.fpp"
  HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/PlatformTypes.h"
  CHOOSES_IMPLEMENTATIONS
    Fw_StringFormat_snprintf
    Fw_StringScan_sscanf
    Os_Task_Stub
    Os_Mutex_Stub
    Os_Console_Stub
    Os_File_Stub
    Os_Cpu_Stub
    Os_Memory_Stub
    Os_RawTime_Stub
    Os_Generic_PriorityQueue
  INTERFACE
  BASE_CONFIG
)
target_compile_definitions(RP2040PlatformTypes INTERFACE -DTGT_OS_TYPE_BAREMETAL)
