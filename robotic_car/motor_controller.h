#include <hardware/gpio.h>
#include <math.h>
#include "hardware/pwm.h"

static const int MOTOR_DIR_FRONT = 0;
static const int MOTOR_DIR_BACK = 1;
static const int MOTOR_DIR_LEFT = 2;
static const int MOTOR_DIR_RIGHT = 3;

static int left_pwm;
static int right_pwm;
static int left_forward;
static int left_backward;
static int right_forward;
static int right_backward;
static int left_encoder;
static int right_encoder;
static uint left_slice;
static uint left_channel;
static uint right_slice;
static uint right_channel;

static int left_ticks;
static int right_ticks;
static int target_ticks;
static int left_pwm_level;
static int right_pwm_level;
static struct repeating_timer adjust_pwm_timer;

float integral = 0;
float previous_error = 0;
float kp = 1.0f;
float ki = 0.1f;
float kd = 0.05f;
float growth = 2.0f;
bool moving = false;

void init_motor_controller(int left_pwm_pin,
                           int right_pwm_pin,
                           int left_forward_pin,
                           int left_backward_pin,
                           int right_forward_pin,
                           int right_backward_pin,
                           int left_encoder_pin,
                           int right_encoder_pin) {

    left_pwm = left_pwm_pin;
    right_pwm = right_pwm_pin;
    left_forward = left_forward_pin;
    left_backward = left_backward_pin;
    right_forward = right_forward_pin;
    right_backward = right_backward_pin;
    left_encoder = left_encoder_pin;
    right_encoder = right_encoder_pin;

    // Configure PWM
    gpio_set_function(left_pwm, GPIO_FUNC_PWM);
    gpio_set_function(right_pwm, GPIO_FUNC_PWM);
    left_slice = pwm_gpio_to_slice_num(left_pwm);
    right_slice = pwm_gpio_to_slice_num(right_pwm);
    left_channel = pwm_gpio_to_channel(left_pwm);
    right_channel = pwm_gpio_to_channel(right_pwm);
    pwm_set_clkdiv(left_slice, 100);
    pwm_set_clkdiv(right_slice, 100);
    pwm_set_wrap(left_slice, 12500);
    pwm_set_wrap(right_slice, 12500);
    pwm_set_chan_level(left_slice, left_channel, 0);
    pwm_set_chan_level(right_slice, right_channel, 0);
    pwm_set_enabled(left_slice, true);
    pwm_set_enabled(right_slice, true);

    // Configure direction GPIO pins
    gpio_set_function(left_forward, GPIO_FUNC_SIO);
    gpio_set_function(left_backward, GPIO_FUNC_SIO);
    gpio_set_function(right_forward, GPIO_FUNC_SIO);
    gpio_set_function(right_backward, GPIO_FUNC_SIO);
    gpio_set_dir(left_forward, GPIO_OUT);
    gpio_set_dir(left_backward, GPIO_OUT);
    gpio_set_dir(right_forward, GPIO_OUT);
    gpio_set_dir(right_backward, GPIO_OUT);


    // Configure encoder input
    gpio_set_function(left_encoder, GPIO_FUNC_SIO);
    gpio_set_function(right_encoder, GPIO_FUNC_SIO);
    gpio_set_dir(left_encoder, GPIO_IN);
    gpio_set_dir(right_encoder, GPIO_IN);
    gpio_set_pulls(left_encoder, true, false);
    gpio_set_pulls(right_encoder, true, false);

}

void set_wheel_direction(int left_dir_forward, int right_dir_forward) {
    gpio_put(left_forward, left_dir_forward);
    gpio_put(left_backward, !left_dir_forward);
    gpio_put(right_forward, right_dir_forward);
    gpio_put(right_backward, !right_dir_forward);

}

void set_wheel_speed(int left_speed, int right_speed) {
    pwm_set_chan_level(left_slice, left_channel, left_speed);
    pwm_set_chan_level(right_slice, right_channel, right_speed);
}

void motor_set_direction(int direction) {

    switch (direction) {
        case MOTOR_DIR_FRONT:
            set_wheel_direction(true, true);
            break;
        case MOTOR_DIR_BACK:
            set_wheel_direction(false, false);
            break;
        case MOTOR_DIR_LEFT:
            set_wheel_direction(false, true);
            break;
        case MOTOR_DIR_RIGHT:
            set_wheel_direction(true, false);
            break;
        default:
            break;
    }

}

void motor_stop() {
    if (!moving)
        return;
    moving = false;
    gpio_set_irq_enabled(left_encoder, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    gpio_set_irq_enabled(right_encoder, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    cancel_repeating_timer(&adjust_pwm_timer);
    set_wheel_speed(0, 0);
    left_ticks = 0;
    right_ticks = 0;
    target_ticks = 0;
    integral = 0;
    previous_error = 0;
}

void wheel_moved_isr(uint gpio, uint32_t events) {

    if (gpio == left_encoder)
        left_ticks++;
    else
        right_ticks++;

    // Stop motor when reached distance
    if (left_ticks >= target_ticks || right_ticks >= target_ticks)
        motor_stop();

}

int compute_pid(int difference) {

    float error = (float)(0 - difference);
    integral += error;
    float derivative = error - previous_error;
    float output = kp * powf(fabsf(error), growth) * (error > 0 ? 1.0f : -1.0f) + ki * integral + kd * derivative;
    previous_error = error;
    return (int)output;

}

bool adjust_pwm_isr(struct  repeating_timer *t) {

    int difference = left_ticks - right_ticks;
    int adjustment = compute_pid(difference);

    left_pwm_level += adjustment;
    right_pwm_level -= adjustment;

    // printf("Left ticks: %d, Right ticks: %d, Left pwm: %d, Right pwm: %d, Difference: %d, Adjustment: %d\n", left_ticks, right_ticks, left_pwm_level, right_pwm_level, difference, adjustment);


    set_wheel_speed(left_pwm_level, right_pwm_level);

    return true;
}


void motor_move(float distance) {

    moving = true;

    // distance / circumference * ticks per rotation
    target_ticks = (int)(distance / 20.41f * 40);

    // Update ticks when wheel moved
    gpio_set_irq_enabled_with_callback(left_encoder, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, wheel_moved_isr);
    gpio_set_irq_enabled_with_callback(right_encoder, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, wheel_moved_isr);

    add_repeating_timer_ms(-100, adjust_pwm_isr, NULL, &adjust_pwm_timer);

    left_pwm_level = 12500;
    right_pwm_level = 12500;
    set_wheel_speed(left_pwm_level, right_pwm_level);

}