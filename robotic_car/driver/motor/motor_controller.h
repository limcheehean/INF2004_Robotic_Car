#include <sys/cdefs.h>
// Import guard for motor controller driver
#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

#include <hardware/pwm.h>
#include "FreeRTOS.h"
#include <task.h>
#include "driver/encoder/wheel_encoder.h"

// Structure to hold PID controller parameters
struct pid
{
    float integral, prev_error, kp, ki, kd;
    int spinning;
};

// Structure to represent a motor
struct motor
{
    int pwm_pin;
    int forward_pin;
    int backward_pin;
    int ticks_per_second;
    int accumulated_ticks;
    uint slice;
    uint channel;
    int pwm_level;
    struct pid pid;

    // target speed each wheel should achieve (the "setpoint" for pid)
    float target_speed; 
};

// Structure to represent a motor driver controlling left and right motors
struct motor_driver
{
    struct motor left_motor;
    struct motor right_motor;
    bool left_motor_status;
    bool right_motor_status;
    int motor_status;
};

// Constants for motor direction and status
static const int FORWARD = 1;
static const int BACKWARD = 0;
static const int MOTOR_STATUS_MOVING = 1;
static const int MOTOR_STATUS_TURNING = -1;
static const int MOTOR_STATUS_STOPPED = 0;

// Function to get the motor configuration
struct motor_driver *get_configuration()
{

    static struct motor_driver config;
    return &config;
};

// Function to update PWM for a motor using PID control
void update_pwm_for_motor(struct motor *motor, struct wheel_encoder *encoder)
{

    // Static count variable for debugging
    static int count = 0;

    // Get pid
    struct pid *pid = &motor->pid;

    // Check current ticks per second
    float current_value = (encoder->ticks - motor->accumulated_ticks); // Check every 100 ms
    float error = motor->target_speed - current_value;                 // target_speed ticks per 100ms
    pid->integral += error;
    float derivative = error - pid->prev_error;
    float control_signal;
    control_signal = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    // Assign curr error as previous
    pid->prev_error = error;

    // Update PWM level
    motor->pwm_level += control_signal;

    // Limit PWM level
    if (motor->pwm_level < 0)
    {
        motor->pwm_level = 0;
    }

    else if (motor->pwm_level > 35000)
    {
        motor->pwm_level = 35000;
    }

    // Update accumulated ticks
    motor->accumulated_ticks = encoder->ticks;
}

// Task to update PWM using PID control for both left and right motors
// Update is done every 10ms
void task_update_pwm_pid()
{

    while (true)
    {

        struct motor_driver *config = get_configuration();
        struct wheel_encoder_data *data = get_encoder_data();

        // Check if both motors are not moving
        if (config->left_motor_status == 0 && config->right_motor_status == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        struct motor *left_motor = &config->left_motor;
        struct motor *right_motor = &config->right_motor;
        struct wheel_encoder *left_encoder = &data->left_encoder;
        struct wheel_encoder *right_encoder = &data->right_encoder;

        // Update PWM for right motor if it is not stopped
        if (config->left_motor_status != MOTOR_STATUS_STOPPED)
        { //&& !config->left_motor.pid.spinning){
            update_pwm_for_motor(left_motor, left_encoder);
        }
        if (config->right_motor_status != MOTOR_STATUS_STOPPED)
        { //&& !config->right_motor.pid.spinning){
            update_pwm_for_motor(right_motor, right_encoder);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task to update motor PWM based on the calculated PWM levels
void task_update_motor_pwm()
{

    while (true)
    {

        struct motor_driver *config = get_configuration();
        struct motor *left_motor = &config->left_motor;
        struct motor *right_motor = &config->right_motor;

        // Check if either left or right motor is stopped
        if (config->left_motor_status == MOTOR_STATUS_STOPPED || config->right_motor_status == MOTOR_STATUS_STOPPED)
        {

            // If left motor is stopped, set its PWM level to 0
            if (config->left_motor_status == MOTOR_STATUS_STOPPED)
            {
                pwm_set_chan_level(left_motor->slice, left_motor->channel, 0);
            }

            // If right motor is stopped, set its PWM level to 0
            if (config->right_motor_status == MOTOR_STATUS_STOPPED)
            {
                pwm_set_chan_level(right_motor->slice, right_motor->channel, 0);
            }

            // Delay for a short period
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        else
        {
            // Set PWM levels for both left and right motors
            pwm_set_chan_level(left_motor->slice, left_motor->channel, left_motor->pwm_level);
            pwm_set_chan_level(right_motor->slice, right_motor->channel, right_motor->pwm_level);
        }

        // Delay for a short period
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Function to initialize the motor controller
void init_motor_controller(int left_pwm_pin,
                           int right_pwm_pin,
                           int left_forward_pin,
                           int right_forward_pin,
                           int left_backward_pin,
                           int right_backward_pin)
{

    // Save pin configuration
    struct motor_driver *config = get_configuration();
    config->motor_status = MOTOR_STATUS_STOPPED;
    config->left_motor_status = MOTOR_STATUS_STOPPED;
    config->right_motor_status = MOTOR_STATUS_STOPPED;
    struct motor *left_motor = &config->left_motor;
    struct motor *right_motor = &config->right_motor;
    left_motor->pwm_pin = left_pwm_pin;
    left_motor->forward_pin = left_forward_pin;
    left_motor->backward_pin = left_backward_pin;
    right_motor->pwm_pin = right_pwm_pin;
    right_motor->forward_pin = right_forward_pin;
    right_motor->backward_pin = right_backward_pin;

    // Configure PID parameters for left and right motors
    left_motor->pid.kp = 200.0f;
    left_motor->pid.ki = 7.5f;
    left_motor->pid.kd = 5.0f;

    right_motor->pid.kp = 220.0f;
    right_motor->pid.ki = 7.5f;
    right_motor->pid.kd = 2.5f;

    // Configure PWM pins for left and right motors
    gpio_set_function(left_pwm_pin, GPIO_FUNC_PWM);
    gpio_set_function(right_pwm_pin, GPIO_FUNC_PWM);
    left_motor->slice = pwm_gpio_to_slice_num(left_pwm_pin);
    right_motor->slice = pwm_gpio_to_slice_num(right_pwm_pin);
    left_motor->channel = pwm_gpio_to_channel(left_pwm_pin);
    right_motor->channel = pwm_gpio_to_channel(right_pwm_pin);
    pwm_set_clkdiv(left_motor->slice, 100);
    pwm_set_clkdiv(right_motor->slice, 100);

    pwm_set_wrap(left_motor->slice, 35000);
    pwm_set_wrap(right_motor->slice, 35000);
    pwm_set_chan_level(left_motor->slice, left_motor->channel, 0);
    pwm_set_chan_level(right_motor->slice, right_motor->channel, 0);
    pwm_set_enabled(left_motor->slice, true);
    pwm_set_enabled(right_motor->slice, true);

    left_motor->target_speed = 5;
    right_motor->target_speed = 5;

    // Configure direction pins
    gpio_set_function(left_forward_pin, GPIO_FUNC_SIO);
    gpio_set_function(left_backward_pin, GPIO_FUNC_SIO);
    gpio_set_function(right_forward_pin, GPIO_FUNC_SIO);
    gpio_set_function(right_backward_pin, GPIO_FUNC_SIO);
    gpio_set_dir(left_forward_pin, GPIO_OUT);
    gpio_set_dir(left_backward_pin, GPIO_OUT);
    gpio_set_dir(right_forward_pin, GPIO_OUT);
    gpio_set_dir(right_backward_pin, GPIO_OUT);

    // Create tasks for updating PWM with PID control and updating motor PWM
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
// Function to normalize motor speed
int64_t normalise_speed()
{
    struct motor_driver *config = get_configuration();
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, config->left_motor.pwm_level);
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, config->right_motor.pwm_level);
    return 0;
}

// Function to set the speed of the right wheel
void set_left_wheel_speed(float left_speed)
{
    struct motor_driver *config = get_configuration();
    config->left_motor.pwm_level = left_speed;
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, left_speed);
}

// Function to set speed for each wheel
void set_right_wheel_speed(float right_speed)
{
    struct motor_driver *config = get_configuration();
    config->right_motor.pwm_level = right_speed;
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, right_speed);
}
// Set speed for each wheel
void set_wheel_speed(float left_speed, float right_speed)
{

    set_left_wheel_speed(left_speed);
    set_right_wheel_speed(right_speed);
}

// Set direction for each wheel
void set_wheel_direction(int left_forward, int right_forward)
{

    struct motor_driver *config = get_configuration();
    gpio_put(config->left_motor.forward_pin, left_forward);
    gpio_put(config->left_motor.backward_pin, !left_forward);
    gpio_put(config->right_motor.forward_pin, right_forward);
    gpio_put(config->right_motor.backward_pin, !right_forward);
}

// Function to set the status of the left motor
void set_left_motor_status(bool status)
{
    struct motor_driver *config = get_configuration();
    config->left_motor_status = status;
    if (config->left_motor_status == MOTOR_STATUS_MOVING)
    {
        config->left_motor.pid.spinning = 0;
        struct motor *left_motor = &config->left_motor;

        left_motor->pwm_level = 0;
        left_motor->accumulated_ticks = 0;
        left_motor->pid.integral = 0.0f;
        left_motor->pid.prev_error = 0.0f;
        left_motor->target_speed = 5;

        struct wheel_encoder_data *data = get_encoder_data();
        data->left_encoder.ticks = 0;
    }
}

// Function to set the status of the right motor
void set_right_motor_status(bool status)
{
    struct motor_driver *config = get_configuration();
    config->right_motor_status = status;

    if (config->right_motor_status == MOTOR_STATUS_MOVING)
    {
        config->right_motor.pid.spinning = 0;
        struct motor *right_motor = &config->right_motor;

        right_motor->pwm_level = 0;
        right_motor->accumulated_ticks = 0;
        right_motor->pid.integral = 0.0f;
        right_motor->pid.prev_error = 0.0f;
        right_motor->target_speed = 5;

        struct wheel_encoder_data *data = get_encoder_data();
        data->right_encoder.ticks = 0;
    }
}

// Function to set the status of both motors
void set_motor_status(int status)
{
    set_left_motor_status(status);
    set_right_motor_status(status);
}
// Move car forward
void move_forward(float left_speed, float right_speed)
{
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, FORWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Move car backward
void move_backward(float left_speed, float right_speed)
{
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(BACKWARD, BACKWARD);
    set_wheel_speed(left_speed, right_speed);
}

// Turn car to the left, about left wheel
void turn_left(float speed)
{
    set_right_motor_status(MOTOR_STATUS_MOVING);
    set_left_motor_status(MOTOR_STATUS_STOPPED);
    struct motor_driver *config = get_configuration();
    config->right_motor.pid.spinning = 1;
    config->right_motor.target_speed = 10;
    set_wheel_direction(FORWARD, FORWARD);
    set_right_wheel_speed(speed);
}

// Turn car to the right, about right wheel
void turn_right(float speed)
{
    set_left_motor_status(MOTOR_STATUS_MOVING);
    set_right_motor_status(MOTOR_STATUS_STOPPED);
    struct motor_driver *config = get_configuration();
    config->left_motor.pid.spinning = 1;
    config->right_motor.target_speed = 10;
    set_wheel_direction(FORWARD, FORWARD);
    set_left_wheel_speed(speed);
}

 // Car moves forward for specified number of ticks
 // (Ticks refer to number of wheel encoder interrupts)
void move_forward_for_ticks(float left_speed, float right_speed, int left_ticks, int right_ticks)
{
    struct wheel_encoder_data *data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
    move_forward(left_speed, right_speed);
}

 // Car reverses for specified number of ticks
 // (Ticks refer to number of wheel encoder interrupts)
void move_backward_for_ticks(float left_speed, float right_speed, int left_ticks, int right_ticks)
{
    move_backward(left_speed, right_speed);
    struct wheel_encoder_data *data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}

 // Turn car to left for specified number of ticks
 // (Ticks refer to number of wheel encoder interrupts)
void turn_left_for_ticks(float speed, int ticks)
{
    struct wheel_encoder_data *data = get_encoder_data();
    data->right_encoder.ticks_to_stop = ticks;
    turn_left(speed);
}

 // Turn car to right for specified number of ticks
 // (Ticks refer to number of wheel encoder interrupts)
void turn_right_for_ticks(float speed, int ticks)
{
    struct wheel_encoder_data *data = get_encoder_data();
    data->left_encoder.ticks_to_stop = ticks;
    turn_right(speed);
}

/**
 * Rotate car to left for specified number of ticks
 * (ticks refer to number of wheel encoder interrupts)
 * 
 * Rotate involves moving both wheels at the same speed
 * with the left wheel reversing
 * */
void rotate_left_for_ticks(float speed, int left_ticks, int right_ticks)
{
    set_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver *config = get_configuration();
    set_wheel_direction(BACKWARD, FORWARD);
    set_wheel_speed(speed, speed);
    struct wheel_encoder_data *data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}

/**
 * Rotate car to right for specified number of ticks
 * (ticks refer to number of wheel encoder interrupts)
 * 
 * Rotate involves moving both wheels at the same speed,
 * with the right wheel reversing
 * */
void rotate_right_for_ticks(float speed, int left_ticks, int right_ticks)
{
    set_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver *config = get_configuration();
    set_wheel_direction(FORWARD, BACKWARD);
    set_wheel_speed(speed, speed);
    struct wheel_encoder_data *data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}

// Rotate about centre
void turn_around(float speed)
{
    set_motor_status(MOTOR_STATUS_MOVING);
    set_wheel_direction(FORWARD, BACKWARD);
    set_wheel_speed(speed, speed);
}

// Stop car
void stop()
{
    set_motor_status(MOTOR_STATUS_STOPPED);

    set_wheel_speed(0, 0);
}

// Function to stop the left wheel
void stop_left_wheel()
{
    set_left_motor_status(MOTOR_STATUS_STOPPED);
}

// Function to stop the right wheel

void stop_right_wheel()
{
    set_right_motor_status(MOTOR_STATUS_STOPPED);
}
#endif
