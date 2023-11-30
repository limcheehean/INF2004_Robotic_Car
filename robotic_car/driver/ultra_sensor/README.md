# Ultrasonic Sensor Code Documentation

## Overview
This repository contains C code for an ultrasonic sensor implementation with a Kalman Filter. The code aims to measure distances and can be integrated into applications requiring distance sensing.

## Hardware Configuration
- Trigger Pin: 19
- Echo Pin: 18

## Dependencies
Ensure the Pico SDK is installed before running the code.

## Configuration Parameters
- NUM_READINGS: Number of readings to average
- MIN_DISTANCE_CM: Minimum distance (in centimeters)
- MAX_DISTANCE_CM: Maximum distance (in centimeters)

## Kalman Filter
The code incorporates a Kalman Filter for smoothing distance measurements.

## Usage Instructions
1. Initialize the sensor using init_ultrasonic().
2. Use ultrasonic_task() in a FreeRTOS task to trigger ultrasonic measurements periodically.
3. Adjust GPIO ISR in echo_pin_isr() based on your application's requirements.

## Example Usage (Testing)
- Uncomment #define ULTRASONIC_TEST for a simple test using main().
- The test initializes the ultrasonic sensor, triggers measurements, and prints results.

## Note
Ensure proper queue handling if used in an embedded system. The code includes a sample integration with a decider module.

Feel free to modify and adapt this code for your specific use case.

You may uncomment the printf() lines for testing and to showcase the ultrasonic readings.
