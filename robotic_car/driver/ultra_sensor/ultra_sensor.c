#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"

#ifndef DECIDER_INCLUDED
#define DECIDER_INCLUDED
#include "../../decider.c"
#endif

/**
 * Pin 13 - trigger
 * Pin 14 - echo
 * **/

#define ULTRA_TRIGGER_PIN 14
#define ULTRA_ECHO_PIN 15

#define NUM_READINGS 3  // Number of readings to average
#define MIN_DISTANCE_CM 6   // Minimum distance (in centimeters)
#define MAX_DISTANCE_CM 100 // Maximum distance (in centimeters)

uint trigPin = 0;
uint echoPin = 1;

volatile uint32_t start_time = 0;
volatile uint32_t end_time = 0;

// Kalman Filter variables
float x_hat = 0.0;  // Estimated best guess of the distance (distance)
float P = 1.0;      // Estimated error covariance (how certain I am about the guess)
float Q = 0.1;      // Process noise covariance
float R = 0.1;      // Measurement noise covariance

void setupUltrasonicPins(uint trigPin, uint echoPin)
{
    gpio_init(ULTRA_TRIGGER_PIN);
    gpio_init(ULTRA_ECHO_PIN);
    gpio_set_dir(ULTRA_TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(ULTRA_ECHO_PIN, GPIO_IN);
}

// Kalman Filter update function from online
void kalmanFilter(float z) {
    // Time update (prediction)
    float x_hat_minus = x_hat;
    float P_minus = P + Q;

    // Measurement update (correction)
    float K = P_minus / (P_minus + R);
    x_hat = x_hat_minus + K * (z - x_hat_minus);
    P = (1 - K) * P_minus;
}


float distance_readings[NUM_READINGS];
int current_reading = 0;

DeciderMessage_t us_decider_message;
BaseType_t holder;
void echo_pin_isr(uint gpio, uint32_t events) {
    if (gpio == ULTRA_ECHO_PIN) {
        if (gpio_get(ULTRA_ECHO_PIN)) {
            // start timer
            start_time = time_us_32();
        } 
        else {
            //end timer
            end_time = time_us_32();
            uint32_t pulse_duration = end_time - start_time;

            // Convert pulse duration to distance in centimeters /29/2
            float distance_cm = pulse_duration / 58; 

            // Use Kalman Filter with the raw distance measurement
            kalmanFilter(distance_cm);

            // Use the filtered distance for further processing by putting into a circular buffer
            float filtered_distance = x_hat;
            
            // Store the distance reading in the circular buffer
            distance_readings[current_reading] = filtered_distance;
            current_reading = (current_reading + 1) % NUM_READINGS;

            // Calculate the average for better reading
            float sum = 0;
            for (int i = 0; i < NUM_READINGS; i++) {
                sum += distance_readings[i];
            }
            float moving_average = sum / NUM_READINGS;

            //if else to print distance. Can use this to move vehicle back.
            if (moving_average >= MAX_DISTANCE_CM ) {
                //printf("\n Too Far!! Distance >%d cm", MAX_DISTANCE_CM);
            }
            else if (moving_average <= MIN_DISTANCE_CM) {
                #ifndef ULTRASONIC_TEST
                printf("\n Too Near!! Distance < %d cm", MIN_DISTANCE_CM);
                us_decider_message.type = D_ULTRASONIC_EVENT;
                us_decider_message.data = 1;
                xQueueSendFromISR(g_decider_message_queue, &us_decider_message, &holder);
                #endif
            }
            else {
                //printf("\n current distance: %.2f cm", moving_average);
            }
        }
    }
}

void ultrasonic_task( void *pvParameters ) {

    for (;;){
        gpio_put(ULTRA_TRIGGER_PIN, 1);  // Set the trigger pin high
        vTaskDelay( pdMS_TO_TICKS(1) /10);          // Keep it high for at least 10 microseconds
        gpio_put(ULTRA_TRIGGER_PIN, 0);  // Set the trigger pin low
        vTaskDelay( pdMS_TO_TICKS(10)); //sleep
    }

}

TaskHandle_t ultrasonic_task_handle;
void init_ultrasonic(){
    setupUltrasonicPins(trigPin, echoPin);
    
    xTaskCreate(ultrasonic_task,
                "Ultrasonic Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) 0, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &ultrasonic_task_handle);
    printf("Ultrasonic initialized\n");
}

#ifdef ULTRASONIC_TEST
int main()
{
    stdio_init_all();
    setupUltrasonicPins(ULTRA_TRIGGER_PIN, ULTRA_ECHO_PIN);

    // Initialize the circular buffer
    for (int i = 0; i < NUM_READINGS; i++) {
        distance_readings[i] = 0;
    }

    // Configure the echo pin for GPIO interrupts
    gpio_set_irq_enabled_with_callback(ULTRA_ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_pin_isr);

    while (true)
    {
        gpio_put(ULTRA_TRIGGER_PIN, 1);  // Set the trigger pin high
        sleep_us(100);          // Keep it high for at least 10 microseconds
        gpio_put(ULTRA_TRIGGER_PIN, 0);  // Set the trigger pin low
        sleep_us(150000); //sleep
    }
}
#endif