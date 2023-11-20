/***
 * Barcode interpret subcomponent
 * 
 * Receives BarcodeISRData_t items from FreeRTOS Queue (sent by barcode_interrupt).
 * 
 * Studies the received values to make sense of which lines are short or longer
 * then interpret the barcode values
 * 
 * */


#ifndef BARCODE_ISR_DATA_HEADER
    #include "barcode_isr_data.h"
    #define BARCODE_ISR_DATA_HEADER 1
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "barcode_buffer.c"

/**
 * For personal reference:
 * taskENTER_CRITICAL();
 * taskEXIT_CRITICAL();
 * 
 * */



TaskHandle_t g_barcode_interpret_task_handle;

BarcodeBuffer_t * get_barcode_buffer(){
    static BarcodeBuffer_t barcode_buffer;
    return &barcode_buffer;
}

void study_interrupt_value(
    BarcodeISRData_t * old_barcode_isr_data_buffer,
    BarcodeISRData_t * barcode_isr_data_buffer){

    uint64_t old_time_passed = old_barcode_isr_data_buffer -> time_passed;
    uint64_t curr_time_passed = barcode_isr_data_buffer -> time_passed;
    if (old_time_passed == 0) {
        /* Very likely, car won't move at the start, so the next barcode will have a very long duration
            Hence, next barcode length will be long.
            There should be a lot of white space before start of barcode, so that is guaranteed long as well.
            1st line of barcode will be short.
        */
        printf("<Barcode length is an estimate for now!>\n");
    }
    if (old_time_passed > curr_time_passed * 2){
            //printf("<Barcode length>\tShort\n");
            barcode_isr_data_buffer->is_short = 1;
        }
    else if (old_time_passed * 2 < curr_time_passed){
            //printf("<Barcode length>\tLong\n");
            barcode_isr_data_buffer->is_short = 0;
        }
    else {
        /* Same as previous */
        if (old_barcode_isr_data_buffer->is_short){
            //printf("<Barcode length>\tShort - past\n");
            barcode_isr_data_buffer->is_short = 1;
        }
        else{
            barcode_isr_data_buffer->is_short = 0;
            //printf("<Barcode length>\tLong - past\n");
        }
    }

    barcode_buffer_put(get_barcode_buffer(), barcode_isr_data_buffer->is_short);
    printf("[");
        
    for(int loop = 0; loop < 10; loop++){
        printf("%d,", barcode_buffer_get(get_barcode_buffer(), loop));
    }

    printf("] -> %ju\n", barcode_isr_data_buffer->time_passed);
        
}

void interpret_barcode(bool * is_reading, BarcodeBuffer_t * barcode_buffer){
    //BarcodeBuffer_t * barcode_buffer = get_barcode_buffer();
    int index_to_copy = 0;
    bool is_start_stop = 1;
    static int start_stop_barcode_fwd[] = {1,0,1,1,0,1,0,1,1};
    static int start_stop_barcode_bwd[] = {1,1,0,1,0,1,1,0,1};

    static int char_A[] =                 {0,1,1,1,1,0,1,1,0};

    bool is_A = 1;
    if (!*is_reading){
        /* To do: Quiet Zone should be used to determine short length*/
        for (int i = BARCODE_BUFFER_READ_OFFSET; i < 10; i++){
            
            if (barcode_buffer_get(barcode_buffer, i) != start_stop_barcode_fwd[i - BARCODE_BUFFER_READ_OFFSET]){
                is_start_stop = 0;
                break;
            }
        }
        if (is_start_stop){
            printf("\n<BARCODE START>\n\n");
            barcode_buffer_clear(barcode_buffer);
            *is_reading = 1;
        }
    }
    else {
        
        for (int i = BARCODE_BUFFER_READ_OFFSET; i < 10; i++){
            if (is_A){
                if (barcode_buffer_get(barcode_buffer, i) != char_A[i - BARCODE_BUFFER_READ_OFFSET]){
                    is_A = 0;
                    //break;
                }
            }
            if (is_start_stop){
                if (barcode_buffer_get(barcode_buffer, i) != start_stop_barcode_fwd[i - BARCODE_BUFFER_READ_OFFSET]){
                    is_start_stop = 0;
                    //break;
                }
            }

            // If array do not contain A for start/stop for sure, stop checking
            if (!is_A && !is_start_stop){
                break;
            }
        }
        if (is_A){
            printf("\n<BARCODE> Letter A detected\n\n");
            barcode_buffer_clear(barcode_buffer);
        }
        else if (is_start_stop){
            printf("\n<STOP BARCODE>\n\n");
            barcode_buffer_clear(barcode_buffer);
            *is_reading = 0;
        }
    }

}

void barcode_interpret_task( void *pvParameters ) {
    BarcodeISRData_t old_barcode_isr_data_buffer;
    BarcodeISRData_t barcode_isr_data_buffer;
    bool is_reading = false;
    //configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    //int buffer = 0;
    for (;;){
        if (xQueueReceive(g_barcode_interpret_queue, &barcode_isr_data_buffer, portMAX_DELAY) == pdPASS){

            study_interrupt_value(&old_barcode_isr_data_buffer, &barcode_isr_data_buffer);
            interpret_barcode(&is_reading, get_barcode_buffer());

            old_barcode_isr_data_buffer = barcode_isr_data_buffer;
        }
        
    }

    vTaskDelete(NULL);
}

void init_barcode_interpret_task(){
    init_barcode_buffer(get_barcode_buffer());
    xTaskCreate(barcode_interpret_task,
                "Barcode Interpret Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) &g_barcode_interpret_queue, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &g_barcode_interpret_task_handle);
    
}