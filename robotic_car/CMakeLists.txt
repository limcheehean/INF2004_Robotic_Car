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