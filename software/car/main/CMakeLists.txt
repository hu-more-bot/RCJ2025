add_executable(main main.c src/imu.c)

target_link_libraries(main pico_stdlib pico_multicore hardware_i2c)
target_include_directories(main PUBLIC "include")

# pico_enable_stdio_usb(main 1)
# pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)
