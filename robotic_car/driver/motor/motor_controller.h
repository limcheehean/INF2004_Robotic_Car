#include <sys/cdefs.h>
// Import guard for motor controller driver
#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

#define KP 50.0f
#define KI 0.0f
#define KD 0.0f

#include <hardware/pwm.h>
#include "FreeRTOS.h"
#include <task.h>
#include "driver/encoder/wheel_encoder.h"


struct pid {
    float integral, prev_error;
};

struct motor {
    int pwm_pin;
    int forward_pin;
    int backward_pin;
    int ticks_per_second;
    int accumulated_ticks;
    uint slice;
    uint channel;
    int pwm_level;
    struct pid pid;
};

struct motor_driver {
    struct motor left_motor;
    struct motor right_motor;
    int motor_status;
};

static const int FORWARD = 1;
static const int BACKWARD = 0;
static const int MOTOR_STATUS_MOVING = 1;
static const int MOTOR_STATUS_TURNING = 2;
static const int MOTOR_STATUS_STOPPED = 3;


struct motor_driver * get_configuration() {

    static struct motor_driver config;
    return &config;

};

void update_pwm_for_motor(struct motor * motor, struct wheel_encoder * encoder) {

   static int count = 0;

   // Get pid
   struct pid * pid = &motor->pid;

   // Check current ticks per second
   float current_value = (encoder->ticks - motor->accumulated_ticks) ; // Check every 100 ms
   //float error = motor->ticks_per_second /10 - current_value;
   float error = (float) encoder -> ticks_to_stop / ( 10 ) - current_value; //10 ticks per 100ms
   //if (motor->accumulated_ticks == 0)
   pid->integral += error;
   float derivative = error - pid->prev_error;
   float control_signal = KP * error + KI * pid->integral + KD * derivative;

   /* Assign curr error as previous */
   pid -> prev_error = error;
   if (motor->pwm_level + control_signal < 0)
       motor->pwm_level = 0;

   motor->pwm_level += control_signal;

   /* Updated ticks */
   motor->accumulated_ticks = encoder->ticks;

   //if (count % 10 == 0)
   //    printf("Total ticks: %d, Current Value: %.2f, Error: %.2f, Control Signal: %.2f, PWM: %d\n", motor->accumulated_ticks, current_value, error, control_signal, motor->pwm_level);
    //if (motor -> ticks_per_second > 0){
    //    printf("Total ticks: %d, Current Value: %.2f, Error: %.2f, Control Signal: %.2f, PWM: %d\n", motor->accumulated_ticks, current_value, error, control_signal, motor->pwm_level);
    
    //}
    printf(" %s: ", encoder->side);
    printf(" Goal/100ms: %d | Goal/s: %d| Current: %d | Motor status: %d |PWM: %d | \n", encoder -> ticks_to_stop / ( 10 ), motor-> ticks_per_second, encoder -> ticks_to_stop, get_configuration()->motor_status,  motor->pwm_level);
}

void task_update_pwm_pid() {

   while (true) {

       struct motor_driver * config = get_configuration();
       struct wheel_encoder_data * data = get_encoder_data();

       if (config->motor_status != MOTOR_STATUS_MOVING)
           continue;

       struct motor * left_motor = &config->left_motor;
       struct motor * right_motor = &config->right_motor;
       struct wheel_encoder * left_encoder = &data->left_encoder;
       struct wheel_encoder * right_encoder = &data->right_encoder;

       update_pwm_for_motor(left_motor, left_encoder);
       update_pwm_for_motor(right_motor, right_encoder);

       vTaskDelay(pdMS_TO_TICKS(100));
   }
}

void task_update_motor_pwm() {

    while (true) {

        struct motor_driver * config = get_configuration();
        struct motor * left_motor = &config->left_motor;
        struct motor * right_motor = &config->right_motor;
        if (config->motor_status != MOTOR_STATUS_MOVING){
            pwm_set_chan_level(left_motor->slice, left_motor->channel, 0);
            pwm_set_chan_level(right_motor->slice, right_motor->channel, 0);
        }
        else{
            pwm_set_chan_level(left_motor->slice, left_motor->channel, left_motor->pwm_level);
            pwm_set_chan_level(right_motor->slice, right_motor->channel, right_motor->pwm_level);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

void init_motor_controller(int left_pwm_pin,
                           int right_pwm_pin,
                           int left_forward_pin,
                           int right_forward_pin,
                           int left_backward_pin,
                           int right_backward_pin) {

    // Save pin configuration
    struct motor_driver * config = get_configuration();
    config->motor_status = MOTOR_STATUS_STOPPED;
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
    pwm_set_wrap(left_motor->slice, 25000);
    pwm_set_wrap(right_motor->slice, 25000);
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

   // Configure PID Task
   xTaskCreate(task_update_pwm_pid,
               "UpdatePwmPidThread",
               configMINIMAL_STACK_SIZE,
               NULL,
               tskIDLE_PRIORITY,
               NULL);
   printf("Create pid task");

   xTaskCreate(task_update_motor_pwm,
               "UpdateMotorPwmThread",
               configMINIMAL_STACK_SIZE,
               NULL,
               tskIDLE_PRIORITY,
               NULL);
   printf("Created update speed test");

}

int64_t normalise_speed() {
    struct motor_driver * config = get_configuration();
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, config->left_motor.pwm_level);
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, config->right_motor.pwm_level);
    return 0;
}

// Set speed for each wheel
void set_wheel_speed(float left_speed, float right_speed) {

    struct motor_driver * config = get_configuration();
//    config->left_motor.ticks_per_second = left_speed * 100;
//    config->right_motor.ticks_per_second = right_speed * 100;
//    config->left_motor.pwm_level = left_speed * 5000;
//    config->right_motor.pwm_level = right_speed * 5000;
    config->left_motor.pwm_level = left_speed;
    config->right_motor.pwm_level = right_speed;
    if (left_speed == 0 && right_speed == 0){
        config->left_motor.ticks_per_second = 0;
        config->left_motor.ticks_per_second = 0;
    }
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, right_speed);

    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, left_speed);
        
    // if (left_speed == 0 && right_speed == 0) {
    //     pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, 0);

    //     pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, 0);
    //     return;
    // }
    // if (left_speed == 0) {
    //     pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, 0);
    //     pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, 20000);
    // } else if (right_speed == 0) {
    //     pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, 20000);
    //     pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, 0);
    // } else {
    //     pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, 20000);
    //     pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, 20000);
    // }
    // add_alarm_in_ms(100, normalise_speed, NULL, false);
}

// Set direction for each wheel
void set_wheel_direction(int left_forward, int right_forward) {

    struct motor_driver * config = get_configuration();
    gpio_put(config->left_motor.forward_pin, left_forward);
    gpio_put(config->left_motor.backward_pin, !left_forward);
    gpio_put(config->right_motor.forward_pin, right_forward);
    gpio_put(config->right_motor.backward_pin, !right_forward);

}

void set_motor_status(int status) {
    struct motor_driver * config = get_configuration();
    config->motor_status = status;

    // Reset PID
    if (status == MOTOR_STATUS_MOVING) {
        struct motor * left_motor = &config->left_motor;
        struct motor * right_motor = &config->right_motor;

        left_motor->pwm_level = 0;
        right_motor->pwm_level = 0;
        left_motor->accumulated_ticks = 0;
        right_motor->accumulated_ticks = 0;
        left_motor->pid.integral = 0.0f;
        right_motor->pid.integral = 0.0f;
        left_motor->pid.prev_error = 0.0f;
        right_motor->pid.prev_error = 0.0f;

        struct wheel_encoder_data * data = get_encoder_data();
        data->left_encoder.ticks = 0;
        data->right_encoder.ticks = 0;
    }
}

// Move car forward
void move_forward(float left_speed, float right_speed) {
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Move car backward
void move_backward(float left_speed, float right_speed) {
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(BACKWARD, BACKWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Turn car to the left, about left wheel
void turn_left(float speed) {
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(0, speed);
}

// Turn car to the right, about right wheel
void turn_right(float speed) {
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(speed, 0);
}

void move_forward_for_ticks(float left_speed, float right_speed, int left_ticks, int right_ticks) {
    move_forward(left_speed, right_speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}

void move_backward_for_ticks(float left_speed, float right_speed, int left_ticks, int right_ticks) {
    move_backward(left_speed, right_speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}

void turn_left_for_ticks(float speed, int ticks) {
    turn_left(speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->right_encoder.ticks_to_stop = ticks;
}

void turn_right_for_ticks(float speed, int ticks) {
    turn_right(speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = ticks;
}

// Rotate about centre
void turn_around(float speed) {
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, BACKWARD);
    set_wheel_speed(speed, speed);
}

// Stop car
void stop() {
    set_motor_status(MOTOR_STATUS_STOPPED);
    set_wheel_speed(0, 0);
}

/*
void stop_left_wheel(){
    struct motor_driver * config = get_configuration();
    config->left_motor.pwm_level = 0;
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, 0);
}
void stop_right_wheel(){
    struct motor_driver * config = get_configuration();
    config->right_motor.pwm_level = 0;
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, 0);
}*/
#endif
