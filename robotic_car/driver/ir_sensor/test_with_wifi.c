
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task_message_buffer.h"
#include "barcode_driver.c"
#include "../wifi/main.c"

#undef BARCODE_UNIT_TESTING
//#define DISABLE_WIFI_MAIN
int main()
{

    
    main_2();

    stdio_init_all();

    /* Declare in main function */
    get_encoder_data() -> left_encoder.current_speed = 1;

    get_encoder_data() -> left_encoder.last_time = 0;
    barcode_driver_init();
    gpio_set_irq_enabled(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(&barcode_edge_irq);


    vTaskStartScheduler(); /* NEED THIS */
    while(true);
}