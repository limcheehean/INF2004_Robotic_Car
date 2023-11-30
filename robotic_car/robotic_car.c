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

// Interrupt Service Routine (ISR) that handles multiple GPIO events
void generic_isr(uint gpio, uint32_t events)
{
    if (gpio == BARCODE_PIN)
    {
        barcode_edge_irq(gpio, events);
    }
    else if (gpio == LEFT_ENCODER_INPUT || gpio == RIGHT_ENCODER_INPUT)
    {
        wheel_moved_isr(gpio, events);
    }
    else if (gpio == LEFT_IR_PIN || gpio == RIGHT_IR_PIN)
    {
        wall_edge_irq(gpio, events);
    }
    else if (gpio == ULTRA_ECHO_PIN)
    {
        echo_pin_isr(gpio, events);
    }
}

// Main function
int main()
{
    /* Initialize drivers and components */
    init_motor();

    // Run the WiFi main function
    main_2();

    barcode_driver_init();

    init_magnetometer();

    init_ultrasonic();

    init_decider();

    // Set up GPIO interrupt callback
    gpio_set_irq_callback(&generic_isr);
    printf("GPIO irq intialized\n");

    // Initialize CGI (Common Gateway Interface)
    cgi_init();
    printf("cgi initialized\n");
    printf("Move forwards!\n");

    // Enable GPIO interrupts for various sensors
    gpio_set_irq_enabled(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(LEFT_ENCODER_INPUT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RIGHT_ENCODER_INPUT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(LEFT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RIGHT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ULTRA_ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    while (true)
    {
    }
}
