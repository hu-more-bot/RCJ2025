cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(rcj2025)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

add_executable(main main.c src/ov7670.c src/motor.c)

# pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/src/clock.pio)

target_link_libraries(main pico_stdlib hardware_i2c hardware_pio hardware_dma hardware_pwm pico_multicore)
target_include_directories(main PUBLIC "include")

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)
