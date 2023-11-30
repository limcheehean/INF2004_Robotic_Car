#ifndef DECIDER_HEADER_
#define DECIDER_HEADER

#include "pico/stdlib.h"

#include "driver/motor/motor_controller.h"
#include "driver/encoder/wheel_encoder.h"
#include "driver/magnetnometer/magnetnometer.c"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Define constants for different types of events
#define D_WALL_LEFT_EVENT 0 // wall detected on left
#define D_WALL_RIGHT_EVENT 1 // wall detected on right
#define D_BARCODE_EVENT 2 //Check if line is barcode
#define D_ULTRASONIC_EVENT 3 // Ultrasonic detects and obstacle
#define D_WALL_TESTING 4 // Event to test for wall
#define D_NOT_BARCODE 5 // Line is detected to be not barcode
#define D_TURNING 6 // Stop car from turning
#define D_NOT_SIDEWALL 7 // Line is detected to not be sidewall
#define D_STOP_REVERSING 9 // Stop car from reversing
#define D_TOGGLE_ULTRASONIC 10 // Toggle ultrasonic event handling

// Structure to represent a message for the decider
typedef struct
{
    int type;
    int data;
} DeciderMessage_t;

// Queue handle for the decider task
QueueHandle_t g_decider_message_queue;

// Function to initialize the decider
void init_decider();

#endif