register_fprime_deployment(pipicube
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/PipiCubeTopology.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
  AUTOCODER_INPUTS
    "${CMAKE_CURRENT_LIST_DIR}/PipiCubeTopology.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/../config/AcConstants.fpp"
  DEPENDS
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

# Generate .bin / .hex / .uf2 alongside the ELF for drag-drop flashing
pico_add_extra_outputs(pipicube)

# USB CDC for stdio (debug); hardware UART reserved for GPS/LoRa
pico_enable_stdio_usb(pipicube 1)
pico_enable_stdio_uart(pipicube 0)
