/***
 * Decider
 * Functions as the "brain" of the part
 * Consists of:
 * - freertos queue: To receive events from drivers
 * - freertos task: To process received events
 * - freertos timers: To perform functions after a set period of time
 * 
 * The decider task may message its own queue to queue an event to handle it later.
 * Events are listed in decider.h
 * */

#include "decider.h"
#include "timers.h"
#include "driver/encoder/wheel_encoder.h"


// Task handle and message queue for the decider task
TaskHandle_t g_decider_task_handle;
QueueHandle_t g_decider_message_queue;

/* An array to hold handles to the created timers. */
TimerHandle_t xTimers[NUM_TIMERS];


/**
 * Timer callback functions below
 * */

// Function to send a message to the decider task
// Queues an event in FreeRTOS task
void message_decider(int type, int data)
{
    DeciderMessage_t message;
    message.type = type;
    message.data = data;
    xQueueSendToFrontFromISR(g_decider_message_queue, &message, 0);
}

void check_wall_callback(TimerHandle_t xTimer)
{
    message_decider(D_NOT_SIDEWALL, 1);
}

void check_barcode_callback(TimerHandle_t xTimer)
{
    message_decider(D_NOT_BARCODE, 1);
}

void stop_reversing_callback(TimerHandle_t xTimer)
{
    message_decider(D_STOP_REVERSING, 1);
}

void turning_callback(TimerHandle_t xTimer)
{
    // get heading
    message_decider(D_TURNING, (int)get_heading());
    turn_left(0.5);
}

void reset_speed_callback(TimerHandle_t xTimer)
{
    get_configuration()->left_motor.target_speed = 3;
    get_configuration()->right_motor.target_speed = 3;
    set_wheel_direction(FORWARD, FORWARD);
    printf("Unhaste\n");
}

// Decider task implementation
/***
 * Decider task
 * The "thinking"/"brain" part of the car
 * */
void decider_task(void *pvParameters)
{

    bool is_reading = false;
    bool ultrasonic_enabled = true;
    DeciderMessage_t message;
    int left_wall_on = 0;
    int right_wall_on = 0;

    int target_heading = 0;
    float speed = 0.5;
    bool i_am_turning = 0;

    /* Timers that can be reset to perform tasks */
    TimerHandle_t rotate_timer = xTimerCreate("Rotating!... ", pdMS_TO_TICKS(100), pdTRUE, (void *)0, turning_callback);
    TimerHandle_t reversing_stop_timer = xTimerCreate("Stop reversing in...", pdMS_TO_TICKS(1000), pdFALSE, (void *)0, stop_reversing_callback);
    TimerHandle_t check_wall_timer = xTimerCreate("check if wall for ...", pdMS_TO_TICKS(150), pdFALSE, (void *)0, check_wall_callback);          
    TimerHandle_t check_barcode_timer = xTimerCreate("check if barcode for ...", pdMS_TO_TICKS(150), pdFALSE, (void *)0, check_barcode_callback); 
    TimerHandle_t reset_speed_timer = xTimerCreate("Reset car speed...", pdMS_TO_TICKS(500), pdFALSE, (void *)0, reset_speed_callback);

    bool calibrated = 0;         /* Need to do a 360 spin */
    bool wall_or_bc_test = 0;    /* Is the wall in front wall or barcode? */
    bool side_or_front_test = 0; /* is line or side or front? */

    int i = 0;
    for (;;)
    {
        if (xQueueReceive(g_decider_message_queue, &message, portMAX_DELAY) == pdPASS)
        {

            switch (message.type)
            {

            /* Stop turning. D_TURNING Event is sent to queue to remind car to stop turning after some time */
            case D_TURNING:
                if (message.data > target_heading)
                {
                    xTimerStop(rotate_timer, portMAX_DELAY);
                    stop();
                }
                break;

            /* sidewall, time to rotate*/
            case D_NOT_SIDEWALL:
                if (left_wall_on || right_wall_on)
                {
                }
                break;
            case D_NOT_BARCODE:
                break;
            case D_ULTRASONIC_EVENT:
                if (ultrasonic_enabled)
                {
                    // stop();
                    ultrasonic_enabled = 0;
                    move_backward_for_ticks(100, 100, 100, 100);
                    xTimerReset(reversing_stop_timer, portMAX_DELAY);
                }
                break;

            case D_TOGGLE_ULTRASONIC:
                ultrasonic_enabled = !message.data;

            case D_STOP_REVERSING:
                xTimerStop(reversing_stop_timer, portMAX_DELAY);
                stop();
                ultrasonic_enabled = 1;
                break;

            /* Upon seeing a wall, modify motor speed to move car away from wall */
            case D_WALL_LEFT_EVENT:
                if (message.data && !right_wall_on)
                {
                    left_wall_on = 1;
                    get_configuration()->left_motor.target_speed = 3;
                    get_configuration()->right_motor.target_speed = 3;
                    set_wheel_direction(FORWARD, BACKWARD);
                    printf("Haste left\n");
                    xTimerReset(reset_speed_timer, portMAX_DELAY);
                }
                if (message.data && right_wall_on)
                {
                    left_wall_on = 1;
                    get_configuration()->left_motor.target_speed = 4;
                    get_configuration()->right_motor.target_speed = 4;
                    set_wheel_direction(BACKWARD, BACKWARD);
                    printf("Reverse\n");
                    xTimerReset(reset_speed_timer, portMAX_DELAY);
                }
                if (!message.data)
                {
                    left_wall_on = 0;
                }
                break;
            case D_WALL_RIGHT_EVENT:
                if (message.data && !left_wall_on)
                {
                    right_wall_on = 1;
                    get_configuration()->right_motor.target_speed = 3;
                    get_configuration()->left_motor.target_speed = 3;
                    set_wheel_direction(BACKWARD, FORWARD);
                    printf("Haste right\n");
                    xTimerReset(reset_speed_timer, portMAX_DELAY);
                }
                if (message.data && left_wall_on)
                {
                    right_wall_on = 1;
                    get_configuration()->right_motor.target_speed = 4;
                    get_configuration()->left_motor.target_speed = 4;
                    set_wheel_direction(BACKWARD, BACKWARD);
                    printf("Backward\n");
                    xTimerReset(reset_speed_timer, portMAX_DELAY);
                }
                if (!message.data)
                {
                    right_wall_on = 0;
                }
                break;
            }
        }
    }
}

// Initialization function for the decider
void init_decider()
{
    g_decider_message_queue = xQueueCreate(30, sizeof(DeciderMessage_t));

    xTaskCreate(decider_task,
                "Decider Task",
                configMINIMAL_STACK_SIZE,
                (void *)&g_decider_message_queue,
                tskIDLE_PRIORITY,
                &g_decider_task_handle);

    printf("Decider initialized\n");
}

// Entry point for the decider test if DECIDER_TEST is defined
#ifdef DECIDER_TEST

int main()
{
    init_decider();
    vTaskStartScheduler(); /* NEED THIS */
    while (true)
        ;
}
#endif
