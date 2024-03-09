cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(plajstik_controller C CXX ASM)
pico_sdk_init()
add_executable(${PROJECT_NAME}
  src/main.c
)

target_sources(${PROJECT_NAME} PRIVATE
  src/main.c
  src/usb_descriptors.c
  src/usb_descriptors.h
  src/tusb_config.h
  include/tiny-json.c
  include/tiny-json.h
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib pico_unique_id tinyusb_device tinyusb_board hardware_adc)
