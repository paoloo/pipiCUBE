# pipicube_os: bare-metal HAL glue only.
# F' Os interface implementations are chosen via CHOOSES_IMPLEMENTATIONS in the
# deployment; the Baremetal/*.cpp files use the v3.x Os interface and are not
# compiled to avoid symbol conflicts with the selected F' stubs.
add_library(pipicube_os INTERFACE)
target_link_libraries(pipicube_os INTERFACE pico_stdlib hardware_sync)
