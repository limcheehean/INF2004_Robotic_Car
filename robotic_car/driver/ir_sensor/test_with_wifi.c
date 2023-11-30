
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task_message_buffer.h"
#include "barcode_driver.c"
#include "../wifi/main.c"

#undef BARCODE_UNIT_TESTING
// #define DISABLE_WIFI_MAIN

// Main function
int main()
{

    // Call the main function from the WiFi module
    main_2();

    // Initialize standard I/O
    stdio_init_all();

    /* Declare in main function */
    // Set initial speed for the left encoder
    get_encoder_data()->left_encoder.current_speed = 1;

    // Set initial time for the left encoder
    get_encoder_data()->left_encoder.last_time = 0;

    // Initialize the barcode driver
    barcode_driver_init();

    // Enable interrupt for BARCODE_PIN with both rising and falling edges
    gpio_set_irq_enabled(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    // Set the interrupt callback function for barcode edge IRQ
    gpio_set_irq_callback(&barcode_edge_irq);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Main loop for continuous execution
    while (true)
        ;
}