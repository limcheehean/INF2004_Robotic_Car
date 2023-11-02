add_executable(ultrasonic_cm getCM.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(ultrasonic_cm pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(ultrasonic_cm 1)
pico_enable_stdio_uart(ultrasonic_cm 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(ultrasonic_cm)

# add url via pico_set_program_url
example_auto_set_url(ultrasonic_cm)