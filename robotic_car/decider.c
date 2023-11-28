
#include "decider.h"
#include "timers.h"
TaskHandle_t g_decider_task_handle;
QueueHandle_t g_decider_message_queue;

/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[ NUM_TIMERS ];

void message_decider(int type, int data){
    DeciderMessage_t message;
    message.type = type;
    message.data = data;
    xQueueSendToFrontFromISR(g_decider_message_queue,&message, 0);
}

void check_wall_callback(TimerHandle_t xTimer){
    message_decider(D_NOT_SIDEWALL, 1 );
}

void check_barcode_callback(TimerHandle_t xTimer){
    message_decider(D_NOT_BARCODE, 1 );
}

void stop_reversing_callback(TimerHandle_t xTimer){
    message_decider(D_STOP_REVERSING, 1);
}

void turning_callback(TimerHandle_t xTimer){
    //get heading
    message_decider(D_TURNING, (int)get_heading());
    turn_left(0.5);
}


void decider_task( void *pvParameters ) {

    bool is_reading = false;
    bool ultrasonic_enabled = true;
    DeciderMessage_t message;
    int left_wall_on = 0;
    int right_wall_on = 0;

    int target_heading = 0;
    float speed = 0.5;
    bool i_am_turning = 0;
    TimerHandle_t rotate_timer = xTimerCreate ( "Rotating!... ", pdMS_TO_TICKS(100), pdTRUE,( void * ) 0,turning_callback);;
    TimerHandle_t reversing_stop_timer = xTimerCreate ( "Stop reversing in...", pdMS_TO_TICKS(1000), pdFALSE,( void * ) 0,stop_reversing_callback);
    TimerHandle_t check_wall_timer = xTimerCreate("check if wall for ...", pdMS_TO_TICKS(150), pdFALSE, (void *) 0, check_wall_callback); //(150, check_wall_isr, 0, true);
    TimerHandle_t check_barcode_timer = xTimerCreate("check if barcode for ...", pdMS_TO_TICKS(150), pdFALSE, (void *) 0, check_barcode_callback); //add_alarm_in_ms(100, check_barcode_isr, 0 ,true);
    
    bool calibrated = 0; /* Need to do a 360 spin */
    bool wall_or_bc_test = 0; /* Is the wall in front wall or barcode? */
    bool side_or_front_test = 0; /* is line or side or front? */

    for (;;){
        if (xQueueReceive(g_decider_message_queue, &message, portMAX_DELAY) == pdPASS){
            
            switch (message.type){
                case D_TURNING:
                    if (message.data > target_heading){
                        xTimerStop(rotate_timer, portMAX_DELAY ); 
                        stop();
                    }
                    break;

                /* sidewall, time to rotate*/
                case D_NOT_SIDEWALL:
                    if (left_wall_on || right_wall_on){
                        //target_heading = get_heading() + 45;
                        //xTimerReset(rotate_timer, portMAX_DELAY);
                        turn_left_for_ticks(100, 23);
                    }
                    break;
                case D_NOT_BARCODE:
                    stop();
                    if (left_wall_on || right_wall_on){
                        //target_heading = get_heading() + 45;
                        //xTimerReset(rotate_timer, portMAX_DELAY);
                         turn_left_for_ticks(100, 23);
                    }
                    break;
                case D_ULTRASONIC_EVENT:
                    if (ultrasonic_enabled){
                        stop();
                        ultrasonic_enabled = 0;
                        move_backward_for_ticks(100, 100, 100, 100);
                        //printf("Reversing\n");
                        //add_alarm_in_ms(500, stop_reversing_isr, NULL, &reversing_stop_alarm);
                        //xTimerReset(reversing_stop_timer,portMAX_DELAY);
                    }
                    break;

                case D_TOGGLE_ULTRASONIC:
                    ultrasonic_enabled = !message.data;

                case D_STOP_REVERSING:
                    xTimerStop(reversing_stop_timer, portMAX_DELAY);
                    stop();
                    ultrasonic_enabled = 1;
                    //printf("Not reversing\n");
                    break;

                    //rotate??
                /* Keep driving forward */
                case D_WALL_LEFT_EVENT: 
                case D_WALL_RIGHT_EVENT:
                /* Currently facing wall */
                    if (i_am_turning) break;
                    /* Bit set */
                    if (message.type == D_WALL_LEFT_EVENT) {
                        if (message.data) left_wall_on = 1;
                        else left_wall_on = 0;
                    }
                    else{
                        if (message.data) right_wall_on = 1;
                        else right_wall_on = 0;
                    }
                    printf(" event data: %d, %d|\n", left_wall_on, right_wall_on);

                    /* One white min */
                    if (! (left_wall_on && right_wall_on )){
                        xTimerStop(check_barcode_timer, portMAX_DELAY);
                        xTimerReset(check_wall_timer, portMAX_DELAY);
                        // Measure angle?
                        // Turn right!
                    }
                    else{
                        // check barcode
                        xTimerStop(check_wall_timer, portMAX_DELAY);
                        xTimerReset(check_barcode_timer, portMAX_DELAY); //= //add_alarm_in_ms(100, check_barcode_isr, 0 ,true);
                    }
                    break;
                
            }
        }
        
    }
    //vTaskDelete(NULL); 
}

void init_decider(){
    g_decider_message_queue = xQueueCreate(30, sizeof(DeciderMessage_t));
    /*
    xTaskCreate(decider_task,
                "Decider Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) &g_decider_message_queue, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &g_decider_task_handle);
    */
    
    printf("Decider initialized\n");
}

#ifdef DECIDER_TEST

int main(){
    init_decider();
    vTaskStartScheduler(); /* NEED THIS */
    while(true);
}
#endif
