// Import guard for motor controller driver
#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

#include <hardware/pwm.h>

struct motor {
    int pwm_pin;
    int forward_pin;
    int backward_pin;
    uint slice;
    uint channel;
};

struct motor_driver {
    struct motor left_motor;
    struct motor right_motor;
};

static const int FORWARD = 1;
static const int BACKWARD = 0;

struct motor_driver * get_configuration() {

    static struct motor_driver config;
    return &config;

};

void init_motor_controller(int left_pwm_pin,
                           int right_pwm_pin,
                           int left_forward_pin,
                           int right_forward_pin,
                           int left_backward_pin,
                           int right_backward_pin) {

    // Save pin configuration
    struct motor_driver * config = get_configuration();
    struct motor * left_motor = &config->left_motor;
    struct motor * right_motor = &config->right_motor;
    left_motor->pwm_pin = left_pwm_pin;
    left_motor->forward_pin = left_forward_pin;
    left_motor->backward_pin = left_backward_pin;
    right_motor->pwm_pin = right_pwm_pin;
    right_motor->forward_pin = right_forward_pin;
    right_motor->backward_pin = right_backward_pin;

    // Configure PWM
    gpio_set_function(left_pwm_pin, GPIO_FUNC_PWM);
    gpio_set_function(right_pwm_pin, GPIO_FUNC_PWM);
    left_motor->slice = pwm_gpio_to_slice_num(left_pwm_pin);
    right_motor->slice = pwm_gpio_to_slice_num(right_pwm_pin);
    left_motor->channel = pwm_gpio_to_channel(left_pwm_pin);
    right_motor->channel = pwm_gpio_to_channel(right_pwm_pin);
    pwm_set_clkdiv(left_motor->slice, 100);
    pwm_set_clkdiv(right_motor->slice, 100);
    pwm_set_wrap(left_motor->slice, 12500);
    pwm_set_wrap(right_motor->slice, 12500);
    pwm_set_chan_level(left_motor->slice, left_motor->channel, 0);
    pwm_set_chan_level(right_motor->slice, right_motor->channel, 0);
    pwm_set_enabled(left_motor->slice, true);
    pwm_set_enabled(right_motor->slice, true);

    // Configure direction pins
    gpio_set_function(left_forward_pin, GPIO_FUNC_SIO);
    gpio_set_function(left_backward_pin, GPIO_FUNC_SIO);
    gpio_set_function(right_forward_pin, GPIO_FUNC_SIO);
    gpio_set_function(right_backward_pin, GPIO_FUNC_SIO);
    gpio_set_dir(left_forward_pin, GPIO_OUT);
    gpio_set_dir(left_backward_pin, GPIO_OUT);
    gpio_set_dir(right_forward_pin, GPIO_OUT);
    gpio_set_dir(right_backward_pin, GPIO_OUT);

}

// Set speed for each wheel
void set_wheel_speed(float left_speed, float right_speed) {

    struct motor_driver * config = get_configuration();
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, (int)(left_speed * 12500));
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, (int)(right_speed * 12500));

}

// Set direction for each wheel
void set_wheel_direction(int left_forward, int right_forward) {

    struct motor_driver * config = get_configuration();
    printf("%d\n", config->right_motor.forward_pin);
    gpio_put(config->left_motor.forward_pin, left_forward);
    gpio_put(config->left_motor.backward_pin, !left_forward);
    gpio_put(config->right_motor.forward_pin, right_forward);
    gpio_put(config->right_motor.backward_pin, !right_forward);

}

// Move car forward
void move_forward(float left_speed, float right_speed) {
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Move car backward
void move_backward(float left_speed, float right_speed) {
    set_wheel_direction(BACKWARD, BACKWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Turn car to the left, about left wheel
void turn_left(float speed) {
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(0, speed);
}

// Turn car to the right, about right wheel
void turn_right(float speed) {
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(speed, 0);
}

// Stop car
void stop() {
    set_wheel_speed(0, 0);
}

#endif
