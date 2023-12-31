add_executable(robotic_car
        robotic_car.c

        driver/wifi/wifi_task_message_receiver.c
        #driver/wifi/wifi_task_message_buffer.h
        #driver/ir_sensor/test_with_wifi.c
        driver/motor/motor_controller.h
        driver/encoder/wheel_encoder.h
        test/motor_test.h
        #driver/motor_test.h
        #demo/demo.c


        #driver/ir_sensor/wall/wall_interrupt.c
        #decider.c



        # Barcode related modules
        # driver/ir_sensor/barcode_driver.c
        )

target_include_directories(robotic_car PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
#        #${CMAKE_CURRENT_LIST_DIR}/../..
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        ${CMAKE_CURRENT_LIST_DIR}/driver/wifi
        )

target_link_libraries(robotic_car
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_i2c
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

target_compile_definitions(robotic_car PRIVATE
        #BARCODE_UNIT_TEST# = 1
        DISABLE_WIFI_MAIN
        -DPICO_MAX_SHARED_IRQ_HANDLERS=5
)

pico_enable_stdio_usb(robotic_car 1)
pico_enable_stdio_uart(robotic_car 0)

pico_add_extra_outputs(robotic_car)


# barcode
add_executable(barcode_driver
        #driver/wifi/main.c
        #driver/wifi/wifi_task_message_receiver.c
        #driver/ir_sensor/barcode_driver.c
        driver/ir_sensor/test_with_wifi.c
        #driver/ir_sensor/barcode_isr_data.h
        #driver/ir_sensor/barcode_interpret.c
        #driver/ir_sensor/barcode_buffer.c
)
target_include_directories(barcode_driver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        ${CMAKE_CURRENT_LIST_DIR}/driver/wifi
        )
target_link_libraries(barcode_driver
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 
        pico_lwip_http
        pico_lwip_iperf

        hardware_pwm
        hardware_adc
        )

target_compile_definitions(barcode_driver PRIVATE
        #BARCODE_UNIT_TEST# = 1
        DISABLE_WIFI_MAIN
        -DPICO_MAX_SHARED_IRQ_HANDLERS=5
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

target_link_libraries(ultrasonic_sensor_driver pico_stdlib hardware_uart hardware_timer

hardware_adc
hardware_pwm
hardware_i2c
FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)
target_include_directories(ultrasonic_sensor_driver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
target_compile_definitions(ultrasonic_sensor_driver PRIVATE
        #BARCODE_UNIT_TEST# = 1
        ULTRASONIC_TEST
        DISABLE_WIFI_MAIN
        -DPICO_MAX_SHARED_IRQ_HANDLERS=5
)

pico_enable_stdio_usb(ultrasonic_sensor_driver 1)
pico_enable_stdio_uart(ultrasonic_sensor_driver 0)

pico_add_extra_outputs(ultrasonic_sensor_driver)

example_auto_set_url(ultrasonic_sensor_driver)



#Wifi

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/makefsdata.py
)

add_executable(pico_w_webserver
        driver/wifi/main.c
        driver/wifi/wifi_task_message_receiver.c
)

#target_compile_definitions(pico_w_webserver1 PRIVATE
#        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
#        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
#        LWIP_TIMEVAL_PRIVATE=0
#        )

target_include_directories(pico_w_webserver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/driver/wifi
#    ${CMAKE_CURRENT_LIST_DIR}/../..
    #${CMAKE_CURRENT_LIST_DIR}/robotic_car/driver/ir_sensor/barcode_subcomponents  # Add this line

)

target_link_libraries(pico_w_webserver
    pico_cyw43_arch_lwip_threadsafe_background
    pico_stdlib
    pico_lwip_iperf
    FreeRTOS-Kernel-Heap4 
    pico_lwip_http
    pico_lwip_iperf
    hardware_adc
    hardware_pwm
)

target_compile_definitions(pico_w_webserver PRIVATE
        
        )

pico_enable_stdio_usb(pico_w_webserver TRUE)
pico_enable_stdio_uart(pico_w_webserver FALSE)

pico_add_extra_outputs(pico_w_webserver)


# decider

add_executable(decider
        decider.c

        driver/wifi/wifi_task_message_receiver.c
        driver/wifi/wifi_task_message_buffer.h
        #driver/ir_sensor/test_with_wifi.c
        driver/motor/motor_controller.h
        driver/encoder/wheel_encoder.h
        test/motor_test.h

        # Barcode related modules
        #driver/ir_sensor/barcode_driver.c
        )

target_include_directories(decider PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/driver/motor_controller/
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        ${CMAKE_CURRENT_LIST_DIR}/driver/wifi
        ${CMAKE_CURRENT_LIST_DIR}/driver/magnetnometer
        )

target_link_libraries(decider
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_i2c
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

target_compile_definitions(decider PRIVATE
        #BARCODE_UNIT_TEST# = 1
        DECIDER_TEST
        DISABLE_WIFI_MAIN
        -DPICO_MAX_SHARED_IRQ_HANDLERS=5
)

pico_enable_stdio_usb(decider 1)
pico_enable_stdio_uart(decider 0)

pico_add_extra_outputs(decider)





# wall

add_executable(wall
        driver/ir_sensor/wall/wall_interrupt.c
        #decider.c

        driver/wifi/wifi_task_message_receiver.c
        driver/wifi/wifi_task_message_buffer.h
        #driver/ir_sensor/test_with_wifi.c
        driver/motor/motor_controller.h
        driver/encoder/wheel_encoder.h
        test/motor_test.h

        # Barcode related modules
        #driver/ir_sensor/barcode_driver.c
        )

target_include_directories(wall PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/driver/motor_controller/
        ${CMAKE_CURRENT_LIST_DIR}/driver/ir_sensor/barcode_subcomponents/
        ${CMAKE_CURRENT_LIST_DIR}/driver/wifi
        ${CMAKE_CURRENT_LIST_DIR}/driver/magnetnometer
        )

target_link_libraries(wall
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_i2c
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

target_compile_definitions(wall PRIVATE
        #BARCODE_UNIT_TEST# = 1
        WALL_UNIT_TEST_
        DISABLE_WIFI_MAIN
        -DPICO_MAX_SHARED_IRQ_HANDLERS=5
)

pico_enable_stdio_usb(wall 1)
pico_enable_stdio_uart(wall 0)

pico_add_extra_outputs(wall)




add_executable(robotic_car_maze
        robotic_car_maze/robotic_car_maze.c
        )

# Import pico_stdlib which imports commonly used features
target_link_libraries(robotic_car_maze pico_stdlib 
        #pico_cyw43_arch_none
        #pico_cyw43_arch_lwip_threadsafe_background
        #pico_lwip_http
        pico_stdlib
        hardware_adc
        hardware_pwm
        hardware_i2c
        FreeRTOS-Kernel-Heap4
        )

target_include_directories(robotic_car_maze PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/robotic_car_maze
)
# Create map/bin/hex file etc.
pico_add_extra_outputs(robotic_car_maze)

# Enable serial output
pico_enable_stdio_usb(robotic_car_maze 1)
pico_enable_stdio_uart(robotic_car_maze 0)

# Add path via pico_set_program_url CMAKE
example_auto_set_url(robotic_car_maze)