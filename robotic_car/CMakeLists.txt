add_executable(robotic_car
        robotic_car.c
        driver/motor/motor_controller.h
        driver/encoder/wheel_encoder.h
        test/motor_test.h

        # Barcode related modules
        #driver/ir_sensor/barcode_driver.c
        )

target_include_directories(robotic_car PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../..
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        )

target_link_libraries(robotic_car
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

pico_enable_stdio_usb(robotic_car 1)
pico_enable_stdio_uart(robotic_car 0)

pico_add_extra_outputs(robotic_car)



add_executable(barcode_driver
        driver/ir_sensor/barcode_driver.c
        #driver/ir_sensor/barcode_isr_data.h
        #driver/ir_sensor/barcode_interpret.c
        #driver/ir_sensor/barcode_buffer.c
)
target_include_directories(barcode_driver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        )
target_link_libraries(barcode_driver
        pico_stdlib
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

target_compile_definitions(barcode_driver PRIVATE
        BARCODE_UNIT_TEST# = 1
)

pico_enable_stdio_usb(barcode_driver 1)
pico_enable_stdio_uart(barcode_driver 0)

pico_add_extra_outputs(barcode_driver)

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

#ultrasonic sensor
add_executable(ultrasonic_sensor_driver driver/ultra_sensor/ultra_sensor.c)

target_link_libraries(ultrasonic_sensor_driver pico_stdlib hardware_uart hardware_timer)

pico_enable_stdio_usb(ultrasonic_sensor_driver 1)
pico_enable_stdio_uart(ultrasonic_sensor_driver 0)

pico_add_extra_outputs(ultrasonic_sensor_driver)

example_auto_set_url(ultrasonic_sensor_driver)