add_executable(robotic_car
        robotic_car.c
        driver/motor/motor_controller.h
        driver/encoder/wheel_encoder.h
        test/motor_test.h
        )

target_include_directories(robotic_car PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )

target_link_libraries(robotic_car
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        )

pico_enable_stdio_usb(robotic_car 1)
pico_enable_stdio_uart(robotic_car 0)

pico_add_extra_outputs(robotic_car)



add_executable(barcode_driver
        driver/ir_sensor/barcode.c
)
target_include_directories(barcode_driver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
target_link_libraries(barcode_driver
        pico_stdlib
        )

pico_enable_stdio_usb(barcode_driver 1)
pico_enable_stdio_uart(barcode_driver 0)

pico_add_extra_outputs(barcode_driver)

#Wifi

set(PROGRAM_NAME pico_w_webserver)
set(PICO_BOARD pico_w)

#include(robotic_car\pico_sdk_import.cmake)
project(pico_w_webserver)

pico_sdk_init()

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_executable(pico_w_webserver
    driver/wifi/main.c
)

target_include_directories(pico_w_webserver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico_w_webserver
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_stdlib
    hardware_adc
    hardware_pwm
)

pico_enable_stdio_usb(pico_w_webserver TRUE)
pico_enable_stdio_uart(pico_w_webserver FALSE)

pico_add_extra_outputs(pico_w_webserver)



# Magnetnometer
add_executable(magnetnometer_driver
        driver/magnetnometer/magnetnometer.c
)
target_include_directories(magnetnometer_driver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
target_link_libraries(magnetnometer_driver
        pico_stdlib
        hardware_i2c
        )

pico_enable_stdio_usb(magnetnometer_driver 1)
pico_enable_stdio_uart(magnetnometer_driver 0)

pico_add_extra_outputs(magnetnometer_driver)