add_library(pipicube_hal STATIC
  rp2040_hal.c
  rp2040_i2c.c
  rp2040_uart.c
  rp2040_time.c
)

target_include_directories(pipicube_hal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pipicube_hal PUBLIC
  pico_stdlib
  hardware_i2c
  hardware_uart
  hardware_gpio
  hardware_irq
  hardware_watchdog
)
