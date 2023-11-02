#include "pico/stdlib.h"
#include <stdio.h>
#include "ultrasonic.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define NUM_READINGS 5  // Number of readings to average
#define MIN_DISTANCE_CM 1   // Minimum distance (in centimeters)
#define MAX_DISTANCE_CM 100 // Maximum distance (in centimeters)

uint trigPin = 0;
uint echoPin = 1;

volatile uint32_t start_time = 0;
volatile uint32_t end_time = 0;

float distance_readings[NUM_READINGS];
int current_reading = 0;

void echo_pin_isr(uint gpio, uint32_t events) {
    if (gpio == echoPin) {
        if (gpio_get(echoPin)) {
            start_time = time_us_32();
        } else {
            end_time = time_us_32();
            uint32_t pulse_duration = end_time - start_time;
            float distance_cm = pulse_duration / 58; // Convert pulse duration to distance in centimeters
            
            // Store the distance reading in the circular buffer
            distance_readings[current_reading] = distance_cm;
            current_reading = (current_reading + 1) % NUM_READINGS;
            
            // Calculate the moving average
            float sum = 0;
            for (int i = 0; i < NUM_READINGS; i++) {
                sum += distance_readings[i];
            }
            float moving_average = sum / NUM_READINGS;

            if (moving_average >= MIN_DISTANCE_CM && moving_average <= MAX_DISTANCE_CM) {
                // Reading is within the desired range, so print it
                printf("\n %f cm", moving_average);
            }
            else {
                printf("Too Close/Too Far!!");
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
        sleep_us(50);          // Keep it high for at least 10 microseconds
        gpio_put(trigPin, 0);  // Set the trigger pin low
        sleep_ms(100); //sleep
    }
}