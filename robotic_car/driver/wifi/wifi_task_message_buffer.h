//wifi_task_message_buffer.h
#ifndef WIFI_TASK_MESSAGE_BUFFER_H
#define WIFI_TASK_MESSAGE_BUFFER_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
#include "../ir_sensor/barcode_subcomponents/barcode_isr_data.h"

#define WIFI_TASK_MESSAGE_SIZE 30

/* Used for communication between Interrupt and Task */
typedef struct {
    int type;
    char message[WIFI_TASK_MESSAGE_SIZE];

} WifiTaskMessage_t;

QueueHandle_t g_wifi_task_message_queue;
WifiTaskMessage_t currentMessage;
//extern SemaphoreHandle_t wifi_data_mutex;
int wifi_shared_type;
void wifi_task_message_receive_task(void *pvParameters);
void init_wifi_task_message_receive();
#endif /* WIFI_TASK_MESSAGE_BUFFER_H */
