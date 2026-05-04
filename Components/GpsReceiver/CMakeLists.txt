set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GpsReceiver.cpp"
)

set(MOD_DEPS
  Fw_Comp
  Fw_Cmd
  Fw_Tlm
  Fw_Log
  Fw_Time
  Svc_Sched
  pipicube_hal
)

register_fprime_module(
  SOURCE_FILES ${SOURCE_FILES}
  DEPENDS      ${MOD_DEPS}
)
