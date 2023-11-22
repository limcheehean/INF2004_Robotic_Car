
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "decider.h"
#include "driver/encoder/wheel_encoder.h"
#include "driver/magnetnometer/magnetnometer.c"

TaskHandle_t g_decider_task_handle;
QueueHandle_t g_decider_message_queue;

void message_self(int type, int data){
    DeciderMessage_t message;
    message.type = type;
    message.data = data;
    xQueueSendToFrontFromISR(g_decider_message_queue,&message, 0);
}

void check_wall_isr(alarm_id_t id, void *user_data){
    message_self(D_WALL_TESTING, 1 );
}

void check_barcode_isr(alarm_id_t id, void *user_data){
    message_self(D_BARCODE_TESTING, 1 );
}

void turning_isr(repeating_timer_t *rt){
    //get heading
    message_self((int)get_heading(), get_heading());
}


void decider_task( void *pvParameters ) {

    bool is_reading = false;
    DeciderMessage_t message;
    int left_wall_sensor = 0;
    int right_wall_sensor = 0;

    int target_heading = 0;
    repeating_timer_t rotate_timer;
    
    bool calibrated = 0; /* Need to do a 360 spin */
    bool wall_or_bc_test = 0; /* Is the wall in front wall or barcode? */
    bool side_or_front_test = 0; /* is line or side or front? */

    for (;;){
        if (xQueueReceive(g_decider_message_queue, &message, portMAX_DELAY) == pdPASS){
            if (message.type == D_TURNING){
                if (message.data > target_heading){
                    cancel_repeating_timer( &rotate_timer ); 
                }
            }
            /* Keep driving forward */
            if (message.type == D_WALL_LEFT_EVENT){
                /* Currently facing wall */
                left_wall_sensor = 1;
                if (right_wall_sensor){
                    add_alarm_in_ms(100, check_wall_isr, 0, true);
                    // Measure angle?
                    // Turn right!
                    
                }
                else {

                }
            }
        }
    }
    
    vTaskDelete(NULL); 
}

void init_decider(){
    g_decider_message_queue = xQueueCreate(30, sizeof(DeciderMessage_t));
}

#ifdef DECIDER_TEST

int main(){
    
}
#endif
