#define INTEGRATED
#include "test/motor_test.h"
#define INCLUDE_WHEEL_ENCODER 1
#include "driver/ir_sensor/barcode_driver.c"
#include "wifi_task_message_buffer.h"

#include "../wifi/main.c"

void generic_isr(uint gpio, uint32_t events) {
    if (gpio == BARCODE_PIN){
        barcode_edge_irq(gpio, events);
    }
    else if (gpio == LEFT_ENCODER_INPUT || gpio == RIGHT_ENCODER_INPUT){
        wheel_moved_isr(gpio, events);
    }
    

}

int main()
{

    //test_motor();

    init_motor();

    main_2();

    barcode_driver_init();

    //float g_shared_dist_buffer = 13212.231;
    //gpio_init(27);
    //gpio_set_dir(27, GPIO_IN);
    //gpio_pull_up(27);

    //gpio_set_irq_enabled_with_callback(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &barcode_edge_irq);
    // Update ticks when wheel moved
    //gpio_set_irq_enabled_with_callback(left_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &wheel_moved_isr);
    //gpio_set_irq_enabled_with_callback(right_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &wheel_moved_isr);

    gpio_set_irq_enabled(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(LEFT_ENCODER_INPUT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RIGHT_ENCODER_INPUT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    gpio_set_irq_callback(&generic_isr);

    cgi_init();

    vTaskStartScheduler(); /* NEED THIS */

    while (true);

}
