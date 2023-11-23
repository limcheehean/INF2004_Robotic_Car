#ifndef DECIDER_HEADER_
#define DECIDER_HEADER

#include "pico/stdlib.h"

#include "driver/motor/motor_controller.h"
#include "driver/encoder/wheel_encoder.h"
#include "driver/magnetnometer/magnetnometer.c"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define D_WALL_LEFT_EVENT 0 
#define D_WALL_RIGHT_EVENT 1
#define D_BARCODE_EVENT 2
#define D_ULTRASONIC_EVENT 3
#define D_WALL_TESTING 4
#define D_NOT_BARCODE 5
#define D_TURNING 6
#define D_NOT_SIDEWALL 7
#define D_STOP_REVERSING 9
#define D_TOGGLE_ULTRASONIC 10

typedef struct {
    int type;
    int data;
} DeciderMessage_t;

QueueHandle_t g_decider_message_queue;
void init_decider();

#endif