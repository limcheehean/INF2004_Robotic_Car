#define INTEGRATED
#include "test/motor_test.h"
#define INCLUDE_WHEEL_ENCODER 1
#include "driver/ir_sensor/barcode_driver.c"
#include "wifi_task_message_buffer.h"

#include "../wifi/main.c"

#include "driver/ir_sensor/wall/wall_interrupt.c"
#define DECIDER_INCLUDED
#include "driver/ultra_sensor/ultra_sensor.c"
#include "robotic_car_maze/maze.c"


void generic_isr(uint gpio, uint32_t events) {
    if (gpio == BARCODE_PIN){
        barcode_edge_irq(gpio, events);
    }
    else if (gpio == LEFT_ENCODER_INPUT || gpio == RIGHT_ENCODER_INPUT){
        wheel_moved_isr(gpio, events);
    }
    else if (gpio == LEFT_IR_PIN || gpio == RIGHT_IR_PIN){
        wall_edge_irq(gpio, events);
    }
    else if (gpio == ULTRA_ECHO_PIN){
        echo_pin_isr(gpio, events);
    }
    

}

int main()
{

    //test_motor();

    init_motor();

    main_2();

    barcode_driver_init();

    init_magnetometer();

    //printf("Magnetometer initialized\n");

    init_ultrasonic();
    
    init_decider();


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
    gpio_set_irq_enabled(LEFT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RIGHT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ULTRA_ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    gpio_set_irq_callback(&generic_isr);


    printf("GPIO irq intialized\n");
    cgi_init();
    printf("cgi initialized\n");

    //move_forward(0.5,0.65);
    //move_forward_for_ticks(8000, 8000, 100, 100);

    printf("Move forwards!\n");

    //init_maze_task();
    //start_mapping();
    //start_navigation();
    
    vTaskStartScheduler(); /* NEED THIS */

    while (true) {
    }

}
