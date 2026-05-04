set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/LoRaDriver.cpp"
)

set(MOD_DEPS
  Fw_Comp
  Fw_Cmd
  Fw_Tlm
  Fw_Log
  Fw_Time
  Svc_Sched
  Fw_Buffer
  pipicube_hal
)

register_fprime_module(
  SOURCE_FILES ${SOURCE_FILES}
  DEPENDS      ${MOD_DEPS}
)
