add_executable(robotic_car_maze
        robotic_car_maze.c
        )

# Import pico_stdlib which imports commonly used features
target_link_libraries(robotic_car_maze pico_stdlib 
        pico_cyw43_arch_none
        #pico_cyw43_arch_lwip_threadsafe_background
        #pico_lwip_http
        #pico_stdlib
        #hardware_adc
        #hardware_pwm
        #hardware_i2c
        #FreeRTOS-Kernel-Heap4
        )

# Create map/bin/hex file etc.
pico_add_extra_outputs(robotic_car_maze)

# Enable serial output
pico_enable_stdio_usb(robotic_car_maze 1)
pico_enable_stdio_uart(robotic_car_maze 0)

# Add path via pico_set_program_url CMAKE
example_auto_set_url(robotic_car_maze)