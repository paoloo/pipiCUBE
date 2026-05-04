add_library(pipicube_os STATIC
  Baremetal/Task.cpp
  Baremetal/Mutex.cpp
  Baremetal/Queue.cpp
  Baremetal/File.cpp
)

target_include_directories(pipicube_os PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pipicube_os PUBLIC
  fprime_Os
  pico_stdlib
  hardware_sync
)
