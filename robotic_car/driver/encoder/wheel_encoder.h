// Import guard to prevent multiple inclusion of this header
#ifndef WHEEL_ENCODER
#define WHEEL_ENCODER

// Include necessary libraries and headers
#include <hardware/gpio.h>
#include <pico/time.h>
#include "pico/stdlib.h"
#include "driver/motor/motor_controller.h"
#include "FreeRTOS.h"
#include "queue.h"

// Function declarations
void stop();
void stop_left_wheel();
void stop_right_wheel();

// Struct to represent a wheel encoder
struct wheel_encoder
{
    int pin;
    int ticks;
    uint64_t last_time;
    float current_speed;
    float total_distance;
    char side[5];
    int ticks_to_stop;
};

// Struct to hold data for both left and right wheel encoders
struct wheel_encoder_data
{
    struct wheel_encoder left_encoder;
    struct wheel_encoder right_encoder;
    QueueHandle_t message_queue;
};

// Function to get a pointer to the wheel encoder data
struct wheel_encoder_data *get_encoder_data()
{
    static struct wheel_encoder_data data;
    return &data;
}

// Interrupt Service Routine (ISR) for wheel movement
void wheel_moved_isr(uint gpio, uint32_t events)
{
    struct wheel_encoder_data *data = get_encoder_data();
    uint64_t current_time = time_us_64();
    // Select the correct encoder based on GPIO
    struct wheel_encoder *encoder = gpio == data->left_encoder.pin ? &data->left_encoder : &data->right_encoder;
    encoder->ticks++;
    if (encoder->ticks >= encoder->ticks_to_stop && encoder->ticks_to_stop > 0)
    {
        int i = 0;
        if (gpio == data->left_encoder.pin)
        {
            i = 1;
            stop_left_wheel();
            xQueueSendFromISR(data->message_queue, &i, NULL);
        }
        else
        {
            i = 2;
            stop_right_wheel();
            xQueueSendFromISR(data->message_queue, &i, NULL);
        }
    }
}

// Initialization function for wheel encoder
void init_wheel_encoder(int left_encoder_pin, int right_encoder_pin)
{
    struct wheel_encoder_data *data = get_encoder_data();

    // Save encoder pins and set the side names
    data->left_encoder.pin = left_encoder_pin;
    strcpy(data->left_encoder.side, "LEFT");
    data->right_encoder.pin = right_encoder_pin;
    strcpy(data->right_encoder.side, "RIGHT");

    // Configure encoder input
    gpio_set_function(left_encoder_pin, GPIO_FUNC_SIO);
    gpio_set_function(right_encoder_pin, GPIO_FUNC_SIO);
    gpio_set_dir(left_encoder_pin, GPIO_IN);
    gpio_set_dir(right_encoder_pin, GPIO_IN);
    gpio_set_pulls(left_encoder_pin, true, false);
    gpio_set_pulls(right_encoder_pin, true, false);
    data->message_queue = xQueueCreate(1, sizeof(int));
}

// End of the import guard
#endif
