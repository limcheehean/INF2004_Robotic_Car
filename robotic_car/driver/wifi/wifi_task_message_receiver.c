#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task_message_buffer.h"

TaskHandle_t g_wifi_task_message_task_handle;

void wifi_task_message_receive_task( void *pvParameters ) {
    bool is_reading = false;
    //configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    WifiTaskMessage_t currentMessage;
    //int buffer = 0;
    for (;;){
        if (xQueueReceive(g_wifi_task_message_queue, &currentMessage, portMAX_DELAY) == pdPASS){

            /* Please remember that printf is not thread safe. Only for unit testing*/
            printf("Type: %d\n", currentMessage.type);
            printf("Message: %s\n", currentMessage.message);
        }
        
    }

    vTaskDelete(NULL);
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