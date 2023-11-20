#include "barcode_interrupt.c"
#include "barcode_interpret.c"

void barcode_driver_init(){
    barcode_interrupt_init();

    g_barcode_interpret_queue = xQueueCreate(10, sizeof(BarcodeISRData_t));
    init_barcode_interpret_task(); 
    
}

#ifdef BARCODE_UNIT_TEST
int main() {

    stdio_init_all();

    /* Declare in main function */
    get_encoder_data() -> left_encoder.current_speed = 1;

    get_encoder_data() -> left_encoder.last_time = 0;
    barcode_driver_init();

    vTaskStartScheduler(); /* NEED THIS */

    /* Unit testing */
    float g_shared_dist_buffer = 13212.231;
    gpio_init(27);
    gpio_set_dir(27, GPIO_IN);
    gpio_pull_up(27);
    gpio_set_irq_enabled_with_callback(27, GPIO_IRQ_LEVEL_LOW /* Must not start with 27 connected to gnd*/, true, &generic_irq);

    

    while (true);
}
#endif