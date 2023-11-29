#include <sys/cdefs.h>
// Import guard for motor controller driver
#ifndef MOTOR_CONTROLLER
#define MOTOR_CONTROLLER

//#define KP 20.0f
//#define KI 1.0f
//#define KD 1.0f

#include <hardware/pwm.h>
#include "FreeRTOS.h"
#include <task.h>
#include "driver/encoder/wheel_encoder.h"


struct pid {
    float integral, prev_error, kp, ki, kd;
    int spinning;
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
    bool left_motor_status;
    bool right_motor_status;
    int motor_status;
};

static const int FORWARD = 1;
static const int BACKWARD = 0;
static const int MOTOR_STATUS_MOVING = 1;
static const int MOTOR_STATUS_TURNING = -1;
static const int MOTOR_STATUS_STOPPED = 0;


struct motor_driver * get_configuration() {

    static struct motor_driver config;
    return &config;

};

void update_pwm_for_motor(struct motor * motor, struct wheel_encoder * encoder) {
        printf("PWM");
        static int count = 0;

        // Get pid
        struct pid * pid = &motor->pid;

        // Check current ticks per second
        float current_value = (encoder->ticks - motor->accumulated_ticks) ; // Check every 100 ms
        //float error = motor->ticks_per_second /10 - current_value;
        //float error = pid -> prev_error - current_value;
        float error = 10 - current_value; //10 ticks per 100ms
        //if (motor->accumulated_ticks == 0)
        pid->integral += error;
        float derivative = error - pid->prev_error;
        float control_signal;
        //if (!motor->pid.spinning){
        control_signal = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
        //}
        /*
        else {
            error = 3 - current_value;
            control_signal = 125 * error + 0.01 * pid->integral + pid->kd/3 * derivative;
        }*/

        /* Assign curr error as previous */
        pid -> prev_error = error;

        motor->pwm_level += control_signal;

        if (motor->pwm_level < 0){
            motor->pwm_level = 0;
        }

        else if (motor -> pwm_level > 25000){
            motor -> pwm_level = 25000;
        }


        /* Updated ticks */
        motor->accumulated_ticks = encoder->ticks;
   //encoder-> accumulated_ticks = 0;

   //if (count % 10 == 0)
   //    printf("Total ticks: %d, Current Value: %.2f, Error: %.2f, Control Signal: %.2f, PWM: %d\n", motor->accumulated_ticks, current_value, error, control_signal, motor->pwm_level);
    //if (motor -> ticks_per_second > 0){
    //    printf("Total ticks: %d, Current Value: %.2f, Error: %.2f, Control Signal: %.2f, PWM: %d\n", motor->accumulated_ticks, current_value, error, control_signal, motor->pwm_level);
    
    //}
    //printf(" %s: ", encoder->side);
    //printf(" Goal/100ms: %d | Current: %.2f | Accumulated: %d|  Motor status: %d |PWM: %d | \n", 5, current_value, motor -> accumulated_ticks, get_configuration()->motor_status,  motor->pwm_level);
}

void task_update_pwm_pid() {

   while (true) {

       struct motor_driver * config = get_configuration();
       struct wheel_encoder_data * data = get_encoder_data();

       if (config->left_motor_status == 0  && config -> right_motor_status == 0){
           //printf("not moving");
           vTaskDelay(pdMS_TO_TICKS(100));
           continue;
       }

       struct motor * left_motor = &config->left_motor;
       struct motor * right_motor = &config->right_motor;
       struct wheel_encoder * left_encoder = &data->left_encoder;
       struct wheel_encoder * right_encoder = &data->right_encoder;
        if (config -> left_motor_status != MOTOR_STATUS_STOPPED && !config->left_motor.pid.spinning){
            //printf("Left not stop|");
            update_pwm_for_motor(left_motor, left_encoder);
        }
        if (config -> right_motor_status != MOTOR_STATUS_STOPPED && !config->right_motor.pid.spinning){
            //printf("Right not stop|");
            update_pwm_for_motor(right_motor, right_encoder);
        }

       vTaskDelay(pdMS_TO_TICKS(100));
   }
}

void task_update_motor_pwm() {

    while (true) {

        struct motor_driver * config = get_configuration();
        struct motor * left_motor = &config->left_motor;
        struct motor * right_motor = &config->right_motor;
        if (config-> left_motor_status == MOTOR_STATUS_STOPPED || config -> right_motor_status == MOTOR_STATUS_STOPPED){
            if (config -> left_motor_status == MOTOR_STATUS_STOPPED){
                pwm_set_chan_level(left_motor->slice, left_motor->channel, 0);
            }
            if (config -> right_motor_status == MOTOR_STATUS_STOPPED){
                pwm_set_chan_level(right_motor->slice, right_motor->channel, 0);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
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
    config->left_motor_status = MOTOR_STATUS_STOPPED;
    config->right_motor_status = MOTOR_STATUS_STOPPED;
    struct motor * left_motor = &config->left_motor;
    struct motor * right_motor = &config->right_motor;
    left_motor->pwm_pin = left_pwm_pin;
    left_motor->forward_pin = left_forward_pin;
    left_motor->backward_pin = left_backward_pin;
    right_motor->pwm_pin = right_pwm_pin;
    right_motor->forward_pin = right_forward_pin;
    right_motor->backward_pin = right_backward_pin;

    /* pid to accomodote for individual motor characteristics */
    left_motor -> pid.kp = 300.0f;
    left_motor -> pid.ki = 7.5f;
    left_motor -> pid.kd = 10.0f;

    right_motor -> pid.kp = 300.0f;
    right_motor -> pid.ki = 5.0f;
    right_motor -> pid.kd = 10.0f;

    // Configure PWM
    gpio_set_function(left_pwm_pin, GPIO_FUNC_PWM);
    gpio_set_function(right_pwm_pin, GPIO_FUNC_PWM);
    left_motor->slice = pwm_gpio_to_slice_num(left_pwm_pin);
    right_motor->slice = pwm_gpio_to_slice_num(right_pwm_pin);
    left_motor->channel = pwm_gpio_to_channel(left_pwm_pin);
    right_motor->channel = pwm_gpio_to_channel(right_pwm_pin);
    pwm_set_clkdiv(left_motor->slice, 100);
    pwm_set_clkdiv(right_motor->slice, 100);
    pwm_set_wrap(left_motor->slice, 62500);
    pwm_set_wrap(right_motor->slice, 62500);

    //pwm_set_wrap(left_motor->slice, 25000);
    //pwm_set_wrap(right_motor->slice, 25000);
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

void set_left_wheel_speed(float left_speed){
    struct motor_driver * config = get_configuration();
    config->left_motor.pwm_level = left_speed;
    pwm_set_chan_level(config->left_motor.slice, config->left_motor.channel, left_speed);   
}

void set_right_wheel_speed(float right_speed){
    struct motor_driver * config = get_configuration();
    config->right_motor.pwm_level = right_speed;
    pwm_set_chan_level(config->right_motor.slice, config->right_motor.channel, right_speed);   
}
// Set speed for each wheel
void set_wheel_speed(float left_speed, float right_speed) {

    set_left_wheel_speed(left_speed);
    set_right_wheel_speed(right_speed);
        
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

void set_left_motor_status(bool status) {
    struct motor_driver * config = get_configuration();
    //config->motor_status = status;
    config-> left_motor_status = status;
    // Not moving = Reset PID
    if (config -> left_motor_status == MOTOR_STATUS_MOVING) {
        config->left_motor.pid.spinning = 0;
        struct motor * left_motor = &config->left_motor;

        left_motor->pwm_level = 0;
        left_motor->accumulated_ticks = 0;
        left_motor->pid.integral = 0.0f;
        left_motor->pid.prev_error = 0.0f;

        struct wheel_encoder_data * data = get_encoder_data();
        data->left_encoder.ticks = 0;
    }
}

void set_right_motor_status(bool status) {
    struct motor_driver * config = get_configuration();
    //config->motor_status = status;
    config-> right_motor_status = status;

    // Not moving = Reset PID
    if (config -> right_motor_status == MOTOR_STATUS_MOVING) {
        config->right_motor.pid.spinning = 0;
        struct motor * right_motor = &config->right_motor;

        right_motor->pwm_level = 0;
        right_motor->accumulated_ticks = 0;
        right_motor->pid.integral = 0.0f;
        right_motor->pid.prev_error = 0.0f;

        struct wheel_encoder_data * data = get_encoder_data();
        data->right_encoder.ticks = 0;
    }
}
void set_motor_status(int status) {
    set_left_motor_status(status);
    set_right_motor_status(status);
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
    set_right_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver * config = get_configuration();
    config->right_motor.pid.spinning = 1;
    set_wheel_direction(FORWARD, FORWARD);
    //set_wheel_speed(0, speed);
    set_right_wheel_speed(speed);
}

// Turn car to the right, about right wheel
void turn_right(float speed) {
    set_left_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver * config = get_configuration();
    config->left_motor.pid.spinning = 1;
    set_wheel_direction(FORWARD, FORWARD);
    //set_wheel_speed(speed, 0);
    set_left_wheel_speed(speed);
}

void move_forward_for_ticks(float left_speed, float right_speed, int left_ticks, int right_ticks) {
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
    move_forward(left_speed, right_speed);
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


void rotate_left_for_ticks(float speed, int left_ticks, int right_ticks){
    set_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver * config = get_configuration();
    //config->left_motor.pid.spinning = 1;
    //config->right_motor.pid.spinning = 1;
    set_wheel_direction(BACKWARD, FORWARD);
    set_wheel_speed(speed, speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
}


void rotate_right_for_ticks(float speed, int left_ticks, int right_ticks){
    set_motor_status(MOTOR_STATUS_MOVING);
    struct motor_driver * config = get_configuration();
    //config->left_motor.pid.spinning = 1;
    //config->right_motor.pid.spinning = 1;
    set_wheel_direction(FORWARD, BACKWARD);
    set_wheel_speed(speed, speed);
    struct wheel_encoder_data * data = get_encoder_data();
    data->left_encoder.ticks_to_stop = left_ticks;
    data->right_encoder.ticks_to_stop = right_ticks;
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


void stop_left_wheel(){
    set_left_motor_status(MOTOR_STATUS_STOPPED);
    //set_left_wheel_speed(0);
}
void stop_right_wheel(){
    set_right_motor_status(MOTOR_STATUS_STOPPED);
    //set_right_wheel_speed(0);
}
#endif
