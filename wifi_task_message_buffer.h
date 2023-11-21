//wifi_task_message_buffer.h
#ifndef WIFI_TASK_MESSAGE_BUFFER_H
#define WIFI_TASK_MESSAGE_BUFFER_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"

#define WIFI_TASK_MESSAGE_SIZE 30

/* Used for communication between Interrupt and Task */
typedef struct {
    int type;
    char message[WIFI_TASK_MESSAGE_SIZE];

} WifiTaskMessage_t;

extern QueueHandle_t g_wifi_task_message_queue;
extern WifiTaskMessage_t currentMessage;
//extern SemaphoreHandle_t wifi_data_mutex;
extern int wifi_shared_type;
extern void wifi_task_message_receive_task(void *pvParameters);
#endif /* WIFI_TASK_MESSAGE_BUFFER_H */
