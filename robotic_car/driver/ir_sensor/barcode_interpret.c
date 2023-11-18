#ifndef BARCODE_ISR_DATA_HEADER
    #include "barcode_isr_data.h"
    #define BARCODE_ISR_DATA_HEADER 1
#endif
#include "task.h"

TaskHandle_t g_barcode_interpret_task_handle;

void barcode_interpret_task( void *pvParameters ) {
    BarcodeISRData_t old_barcode_isr_data_buffer;
    BarcodeISRData_t barcode_isr_data_buffer;
    uint64_t counter = 0;
    //configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    //int buffer = 0;
    for (;;){
        if (xQueueReceive(g_barcode_interpret_queue, &barcode_isr_data_buffer, portMAX_DELAY) == pdPASS){
            //printf("<B Interpret> Received %d\n", barcode_isr_data_buffer.is_short);
            //printf("<B Intepret> Counter at %ju\n", ++counter);
        }
        
    }

    vTaskDelete(NULL);
}

void init_barcode_interpret_task(){
    xTaskCreate(barcode_interpret_task,
                "Barcode Interpret Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) &g_barcode_interpret_queue, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &g_barcode_interpret_task_handle);
}