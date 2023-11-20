cmake_minimum_required(VERSION 3.12)

# Import Pico SDK
include(pico_sdk_import.cmake)


project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

# Initialise FreeRTOS
include(FreeRTOS_Kernel_import.cmake)

include(example_auto_set_url.cmake)

# Add main Folder
add_subdirectory(robotic_car)

#Wifi

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/makefsdata.py
)

add_executable(pico_w_webserver1
        main.c
)

#target_compile_definitions(pico_w_webserver1 PRIVATE
#        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
#        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
#        LWIP_TIMEVAL_PRIVATE=0
#        )

target_include_directories(pico_w_webserver1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
#    ${CMAKE_CURRENT_LIST_DIR}/../..
    #${CMAKE_CURRENT_LIST_DIR}/robotic_car/driver/ir_sensor/barcode_subcomponents  # Add this line

)

target_link_libraries(pico_w_webserver1
    pico_cyw43_arch_lwip_threadsafe_background
    pico_stdlib
    pico_lwip_iperf
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    pico_lwip_http
    pico_lwip_iperf
    hardware_adc
)

pico_enable_stdio_usb(pico_w_webserver1 TRUE)
pico_enable_stdio_uart(pico_w_webserver1 FALSE)

pico_add_extra_outputs(pico_w_webserver1)