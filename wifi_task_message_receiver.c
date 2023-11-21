
#include "FreeRTOS.h"
#include "task.h"
#ifndef WIFI_TASK_MESSAGE_HEADER
    #include "wifi_task_message_buffer.h"
    #define WIFI_TASK_MESSAGE_HEADER 1
#endif
#include "robotic_car/driver/ir_sensor/barcode_subcomponents/barcode_isr_data.h"

TaskHandle_t g_wifi_task_message_task_handle;
WifiTaskMessage_t currentMessage;
QueueHandle_t g_wifi_task_message_queue;

void wifi_task_message_receive_task( void *pvParameters ) {
    bool is_reading = false;

    //Testing currentMessage object value before sending over to SSI.
    // currentMessage.type = 41;
    // snprintf(currentMessage.message, WIFI_TASK_MESSAGE_SIZE, "Hello, Task!");
    // printf("Type: %d\n", currentMessage.type);
    // printf("Message: %s\n", currentMessage.message);
    // Send the initial values to the queue
    //xQueueSend(g_wifi_task_message_queue, &currentMessage, 0); 


    for (;;){
        if (xQueueReceive(g_wifi_task_message_queue, &currentMessage, portMAX_DELAY) == pdPASS){
            /* Please remember that printf is not thread safe. Only for unit testing*/
            //Can remove one finished testing. Passing currentMessage object directly to SSI
            printf("Type: %d\n", currentMessage.type);
            printf("Message: %s\n", currentMessage.message);
            }
        }
        vTaskDelete(NULL); 
    }

void init_barcode_intr_queue(QueueHandle_t * wifi_task_message_queue){
    g_wifi_task_message_queue =  *wifi_task_message_queue;
}

void init_wifi_task_message_receive(){
    init_barcode_buffer(get_barcode_buffer());
    xTaskCreate(wifi_task_message_receive_task,
                "Barcode Interpret Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) 0, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &g_wifi_task_message_task_handle);
    
}

