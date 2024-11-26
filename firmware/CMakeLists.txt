cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake)

project(plajstik_controller C CXX ASM)

pico_sdk_init()
add_executable(${PROJECT_NAME}
  src/main.c
)

target_sources(${PROJECT_NAME} PRIVATE
  src/main.c
  src/usb_descriptors.c
  src/tiny-json.c
  src/maps.c
  src/utils.c
  src/tusb_callbacks.c
  include/usb_descriptors.h
  include/tusb_config.h
  include/tiny-json.h
  include/maps.h
  include/utils.h
  include/tusb_callbacks.h
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib pico_unique_id tinyusb_device tinyusb_board hardware_adc hardware_sync)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)
