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
#include "barcode_interpret.h"
#include "wifi_task_message_buffer.h"


TaskHandle_t g_barcode_interpret_task_handle;

uint16_t g_barcode_buffer = 0x0;

QueueHandle_t g_barcode_interpret_queue;

uint16_t reverse_binary(uint16_t value, int size){

    uint16_t reversed_value = 0;

    for (unsigned int i = 0; i < BARCODE_BUFFER_SIZE; i++) {

        /* For every bit in buffer, if bit is 1 */
        if (value & (1 << i)) {
            /* Use bitwise or to set binary values from the back */
            reversed_value |= (1 << ((BARCODE_BUFFER_SIZE - 1) - i));
        }
    }

    return reversed_value;
}

/**
 * Differentiate between thin and thick line
 * This is done by checking if :
 * Twice the thickness of current line is smaller than previous line
 * (if true, line is thin)
 * or 
 * Twice the thickness of previous line is smaller than current line
 * (if true, line is thick)
 * */

void study_interrupt_value(
    BarcodeISRData_t * old_isr_data,
    BarcodeISRData_t * new_isr_data,
    uint16_t * barcode_buffer_ptr,
    bool * next_is_quiet,
    bool * refresh_buffers){

    static int count = 0; /* How many bars so far */

    //int offset_sign = 1; 

    float old_length = old_isr_data -> time_passed; //* old_isr_data -> wheel_encoder_speed;
    float new_length = new_isr_data -> time_passed; //* new_isr_data ->  wheel_encoder_speed;
    if (old_length == 0) {
        /* Very likely, car won't move at the start, so the next barcode will have a very long duration
            Hence, next barcode length will be long.
            There should be a lot of white space before start of barcode, so that is guaranteed long as well.
            1st line of barcode will be short.
        */
        printf("<Barcode length is an estimate for now!>\n");
    }
    if (old_length > new_length * 2){
            //printf("<Barcode length>\tShort\n");
            new_isr_data->is_short = 1;
        }
    else if (old_length * 2 < new_length){
            //printf("<Barcode length>\tLong\n");
            new_isr_data->is_short = 0;
        }
    else {
        /* Same as previous */
        if (old_isr_data->is_short){
            //printf("<Barcode length>\tShort - past\n");
            new_isr_data->is_short = 1;
        }
        else{
            new_isr_data->is_short = 0;
            //printf("<Barcode length>\tLong - past\n");
        }
    }
    /* Quiet zone, don't push */
    if (*next_is_quiet){
        new_isr_data->is_short = 1;
        *next_is_quiet = false;
        *barcode_buffer_ptr = 0; //clear buffer for letters
    }
    if (* refresh_buffers){
        *barcode_buffer_ptr = 0;
        * refresh_buffers = false;
    }
    else{
        *barcode_buffer_ptr = (*barcode_buffer_ptr << 1) + new_isr_data->is_short;
    }
        
        #ifndef NOT_DEBUGGING
        printf("[");
        
        for(int loop = 0; loop < BARCODE_BUFFER_SIZE; loop++){
            printf("%d,", (*barcode_buffer_ptr >> BARCODE_BUFFER_SIZE -1 - loop) & 1  );
        }

        printf("] \n");/*-> Average of %f\n", average);*/
        #endif
    }
        

/* "Read" the barcode buffer for characters */
void interpret_barcode( uint16_t barcode_buffer, bool * next_is_quiet, bool * refresh_buffers){
    
    int index_to_copy = 0;
    bool is_start_stop = 0;
    static bool is_reversed = 0;
    static bool is_reading = 0;
    static WifiTaskMessage_t new_message;

    new_message.type = 1;
    bool is_A = 1;

    /* Checking for start stop character only */
    if (!is_reading){
        /* To do: Quiet Zone should be used to determine short length*/

        if ((barcode_buffer & BC_START_STOP) == BC_START_STOP){
            is_start_stop = 1;
            is_reversed = 0;
        }
        else {
            barcode_buffer = reverse_binary(barcode_buffer, BARCODE_BUFFER_SIZE);
            if ((barcode_buffer & BC_START_STOP) == BC_START_STOP){
                is_start_stop = 1;
                is_reversed = 1;
            }
        }
    
        if (is_start_stop){

            //new_message.message[0] = '*';
            //xQueueSend(g_wifi_task_message_queue, &new_message, 0); 

            #ifndef NOT_DEBUGGING
            printf("\n<BARCODE START>\n\n");
            #endif
            * refresh_buffers = true;
            is_reading = true;
            *next_is_quiet = true;

            is_start_stop = 0;
        }
    }

    /* Start checking for all characters */
    else {

        /* Reverse barcode buffer if needed */
        if (is_reversed){
            barcode_buffer = reverse_binary(barcode_buffer, BARCODE_BUFFER_SIZE);
        } 
        if ((barcode_buffer & BC_START_STOP) == BC_START_STOP){
            
            //new_message.message[0] = '*';
            //xQueueSend(g_wifi_task_message_queue, &new_message, 0); 
            #ifndef NOT_DEBUGGING
            printf("\n<BARCODE STOP>\n\n");
            #endif
            // clear barcode buffer
            * refresh_buffers = true;
            * next_is_quiet = true;

            /* Reset function states */
            is_reading = 0;
            is_reversed = 0;
        }
        else{
            /* Check through all characters */
            char c = get_barcode_char(barcode_buffer);
            if (c != ' '){

                new_message.message[0] = c;
                xQueueSend(g_wifi_task_message_queue, &new_message, 0); 
                #ifndef NOT_DEBUGGING
                printf(" \n READ %c\n",c);
                #endif

                /* Send to web server */

                /* Signal to clear buffer */
                * refresh_buffers = true;
            }


        }

    }

}

void barcode_interpret_task( void *pvParameters ) {
    
    BarcodeISRData_t old_isr_data;
    BarcodeISRData_t new_isr_data;
    bool is_reading = false;

    bool refresh_buffers = false;
    bool next_is_quiet = false;

    for (;;){
        if (xQueueReceive(g_barcode_interpret_queue,&new_isr_data, portMAX_DELAY) == pdPASS){
            study_interrupt_value(&old_isr_data, &new_isr_data, &g_barcode_buffer, &next_is_quiet, &refresh_buffers);
            interpret_barcode(g_barcode_buffer, &next_is_quiet, &refresh_buffers);
            old_isr_data=new_isr_data;   
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