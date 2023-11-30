add_executable(robotic_car_maze_nocar
        robotic_car_maze.c
        )

# Import pico_stdlib which imports commonly used features
target_link_libraries(robotic_car_maze_nocar pico_stdlib pico_cyw43_arch_none)

# Create map/bin/hex file etc.
pico_add_extra_outputs(robotic_car_maze_nocar)

# Enable serial output
pico_enable_stdio_usb(robotic_car_maze_nocar 1)
pico_enable_stdio_uart(robotic_car_maze_nocar 0)

# Add path via pico_set_program_url CMAKE
example_auto_set_url(robotic_car_maze_nocar)