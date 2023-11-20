#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define NUM_READINGS 5  // Number of readings to average
#define MIN_DISTANCE_CM 3   // Minimum distance (in centimeters)
#define MAX_DISTANCE_CM 100 // Maximum distance (in centimeters)

uint trigPin = 0;
uint echoPin = 1;

volatile uint32_t start_time = 0;
volatile uint32_t end_time = 0;

// Kalman Filter variables
float x_hat = 0.0;  // Estimated state (distance)
float P = 1.0;      // Estimated error covariance
float Q = 0.1;      // Process noise covariance
float R = 0.1;      // Measurement noise covariance

void setupUltrasonicPins(uint trigPin, uint echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
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

void echo_pin_isr(uint gpio, uint32_t events) {
    if (gpio == 1) {
        if (gpio_get(echoPin)) {
            // start timer
            start_time = time_us_32();
        } 
        else {
            //end timer
            end_time = time_us_32();
            uint32_t pulse_duration = end_time - start_time;
            // Convert pulse duration to distance in centimeters /29/2
            float distance_cm = pulse_duration / 58; 

            // Update Kalman Filter with the raw distance measurement
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

            if (moving_average >= MAX_DISTANCE_CM ) {
                printf("\n Too Far!! Distance >%d cm", MAX_DISTANCE_CM);
            }
            else if (moving_average <= MIN_DISTANCE_CM) {
                printf("\n Too Near!! Distance < %d cm", MIN_DISTANCE_CM);
            }
            else {
                printf("\n current distance: %.2f cm", moving_average);
            }
        }
    }
}

int main()
{
    stdio_init_all();
    setupUltrasonicPins(trigPin, echoPin);

    // Initialize the circular buffer
    for (int i = 0; i < NUM_READINGS; i++) {
        distance_readings[i] = 0;
    }

    // Configure the echo pin for GPIO interrupts
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_pin_isr);

    while (true)
    {
        gpio_put(trigPin, 1);  // Set the trigger pin high
        sleep_us(100);          // Keep it high for at least 10 microseconds
        gpio_put(trigPin, 0);  // Set the trigger pin low
        sleep_ms(150); //sleep
    }
}