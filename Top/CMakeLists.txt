register_fprime_module()

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/PipiCubeTopology.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

set(MOD_DEPS
  PipiCube_BatteryMonitor
  PipiCube_GpsReceiver
  PipiCube_LoRaDriver
  Svc_ActiveRateGroup
  Svc_RateGroupDriver
  Svc_CmdDispatcher
  Svc_TlmChan
  Svc_ActiveLogger
  Svc_Time
  pipicube_hal
  pipicube_os
  pico_stdlib
  hardware_timer
)

add_fprime_target(pipicube
  SOURCES  ${SOURCE_FILES}
  DEPENDS  ${MOD_DEPS}
)

# Generate .uf2 for drag-drop flashing
pico_add_extra_outputs(pipicube)

# Enable USB serial for stdio (optional, disable in flight config)
pico_enable_stdio_usb(pipicube 1)
pico_enable_stdio_uart(pipicube 0)
