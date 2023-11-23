
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task_message_buffer.h"
#include "string.h"
#define MAX_MESSAGES 100

TaskHandle_t g_wifi_task_message_task_handle;
TaskHandle_t g_wifi_task_message_task_handle_test;
WifiTaskMessage_t currentMessage;
WifiTaskMessage_t totalMessage;
QueueHandle_t g_wifi_task_message_queue;
QueueHandle_t g_concatenatedMessagesQueue;

void wifi_task_message_receive_task(void *pvParameters)
{
    bool is_reading = false;

    for (;;)
    {
        if (xQueueReceive(g_wifi_task_message_queue, &currentMessage, portMAX_DELAY) == pdPASS)
        {
            /* Please remember that printf is not thread safe. Only for unit testing */
            printf("Type: %d\n", currentMessage.type);
            printf("Message: %s\n", currentMessage.message);

            totalMessage.type = currentMessage.type;

            if (strlen(totalMessage.message) + strlen(currentMessage.message) < WIFI_TASK_MESSAGE_SIZE)
            {
                // Concatenate currentMessage.message to totalMessage.message
                strncat(totalMessage.message, currentMessage.message, WIFI_TASK_MESSAGE_SIZE - strlen(totalMessage.message) - 1);
                printf("Concatenated message: %s\n", totalMessage.message);
            }
            else
            {
                printf("Not enough space to concatenate the message.\n");
            }

            xQueueSend(g_concatenatedMessagesQueue, &totalMessage, 0);
        }
    }

    // Task will only reach here if the loop exits, indicating no more messages to process
    vTaskDelete(NULL);
}

#ifndef DISABLE_WIFI_MAIN
void wifi_task_message_receive_task_testData(void *pvParameters)
{

    for (;;)
    {
        // Modify the message field before sending
        snprintf(currentMessage.message, WIFI_TASK_MESSAGE_SIZE, "H%d", currentMessage.type);

        printf("Type: %d\n", currentMessage.type);
        printf("Message: %s\n", currentMessage.message);

        xQueueSend(g_wifi_task_message_queue, &currentMessage, 0);
        vTaskDelay(3000);
    }

    vTaskDelete(NULL);
}
#endif
void init_wifi_intr_queue(QueueHandle_t *wifi_task_message_queue)
{
    g_wifi_task_message_queue = *wifi_task_message_queue;
}

// void init_wifi_task_message_receive(){
//     //init_barcode_buffer(get_barcode_buffer());
//     g_wifi_task_message_queue =  xQueueCreate(5, sizeof(WifiTaskMessage_t));
//     xTaskCreate(wifi_task_message_receive_task,
//                 "Barcode Interpret Task",
//                 configMINIMAL_STACK_SIZE,
//                 ( void * ) 0, // Can try experimenting with parameter
//                 tskIDLE_PRIORITY,
//                 &g_wifi_task_message_task_handle);
//     printf("task created");

// }

// void init_wifi_task_message_receive_test(){
//     //init_barcode_buffer(get_barcode_buffer());
//     g_concatenatedMessagesQueue = xQueueCreate(1, sizeof(MAX_MESSAGES));
//     xTaskCreate(wifi_task_message_receive_task_test,
//                 "Barcode Interpret Task Test",
//                 configMINIMAL_STACK_SIZE,
//                 ( void * ) 0, // Can try experimenting with parameter
//                 tskIDLE_PRIORITY,
//                 &g_wifi_task_message_task_handle_test);
//     printf("test task created");

// }
