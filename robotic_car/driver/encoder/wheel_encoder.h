

#include <hardware/gpio.h>
#include <pico/time.h>
#include "pico/stdlib.h"


struct wheel_encoder {

    int pin;
    int ticks;
    uint64_t last_time;
    float current_speed;
    float total_distance;
    char side[5];

};

struct wheel_encoder_data {

    struct wheel_encoder left_encoder;
    struct wheel_encoder right_encoder;

};

struct wheel_encoder_data * get_encoder_data() {

    static struct wheel_encoder_data data;
    return &data;

}

void wheel_moved_isr(uint gpio, uint32_t events) {

    struct wheel_encoder_data * data = get_encoder_data();
    uint64_t current_time = time_us_64();

    // Select the correct encoder based on GPIO
    struct wheel_encoder * encoder = gpio == data->left_encoder.pin ? &data->left_encoder : &data->right_encoder;

    encoder->ticks++;
    encoder->total_distance = (float)encoder->ticks / 40 * 6.5f;
    encoder->current_speed = 6.5f / 40 / ((float)(current_time - encoder->last_time) / 1000000.0f);
    encoder->last_time = current_time;

    printf("Encoder: %s, Distance: %.2f cm, Speed: %.2f cm/s\n", encoder->side, encoder->total_distance, encoder->current_speed);

}

void init_wheel_encoder(int left_encoder_pin, int right_encoder_pin) {

    // Save encoder pins
    struct wheel_encoder_data * data = get_encoder_data();
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

    // Update ticks when wheel moved
    gpio_set_irq_enabled_with_callback(left_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &wheel_moved_isr);
    gpio_set_irq_enabled_with_callback(right_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &wheel_moved_isr);

}