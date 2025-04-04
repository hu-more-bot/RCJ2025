add_executable(main main.c src/motor.c)

target_link_libraries(main pico_stdlib pico_multicore hardware_pwm)
target_include_directories(main PUBLIC "include")

# pico_enable_stdio_usb(main 1)
# pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)

# imu test
add_executable(imu imu.c)
target_link_libraries(imu pico_stdlib hardware_spi)
pico_enable_stdio_usb(imu 1)
pico_enable_stdio_uart(imu 0)
pico_add_extra_outputs(imu)

# cam test
add_executable(cam cam.c)
target_link_libraries(cam pico_stdlib hardware_i2c)
pico_enable_stdio_usb(cam 1)
pico_enable_stdio_uart(cam 0)
pico_add_extra_outputs(cam)
