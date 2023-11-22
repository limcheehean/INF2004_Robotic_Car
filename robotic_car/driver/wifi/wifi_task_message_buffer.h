// wifi_task_message_buffer.h
#ifndef WIFI_TASK_MESSAGE_BUFFER_H
#define WIFI_TASK_MESSAGE_BUFFER_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
#include "../ir_sensor/barcode_subcomponents/barcode_isr_data.h"

#define WIFI_TASK_MESSAGE_SIZE 30
#define MAX_MESSAGES 100

/* Used for communication between Interrupt and Task */
typedef struct
{
    int type;
    char message[WIFI_TASK_MESSAGE_SIZE];

} WifiTaskMessage_t;

extern QueueHandle_t g_wifi_task_message_queue;
extern WifiTaskMessage_t currentMessage;
extern WifiTaskMessage_t totalMessage;
extern int wifi_shared_type;
extern void wifi_task_message_receive_task(void *pvParameters);
extern void wifi_task_message_receive_task_testData(void *pvParameters);

extern char concatenatedMessages[WIFI_TASK_MESSAGE_SIZE * MAX_MESSAGES + 1];
extern QueueHandle_t g_concatenatedMessagesQueue;
extern QueueHandle_t g_update_ssi_queue;
extern TaskHandle_t g_wifi_task_message_task_handle;
extern TaskHandle_t g_wifi_task_message_task_handle_test;

#endif /* WIFI_TASK_MESSAGE_BUFFER_H */
