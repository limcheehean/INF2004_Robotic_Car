#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"

#define WIFI_TASK_MESSAGE_SIZE 30

/* Used for communication between Interrupt and Task */
typedef struct {
    int type;
    char message[WIFI_TASK_MESSAGE_SIZE];

} WifiTaskMessage_t;

QueueHandle_t g_wifi_task_message_queue;