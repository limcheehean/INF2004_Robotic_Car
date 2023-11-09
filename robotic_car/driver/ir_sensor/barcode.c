/**
 * Interrupt-based ir line sensor
 * Usage instruction:
 *  - Connect BARCODE_PIN (Pin 9) to Analogue Pin of IR Line Sensor
 *      Reason: Digital pin may trigger interrupt of EDGE RISE and EDGE LOW at same time (event mask will include both high and low in 1 interrupt)
 *      Each interrupt from Analogue pin will only be EDGE RISE or EDGE LOW, not both at same time
 *      This is needed if the car moves too fast.
 * - Debounce is not required, tested with no issues so far
 * */

#include "pico/stdlib.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define BARCODE_PIN 9

uint32_t last_press_time = 0;  // Store the last time the button was pressed

// Store old distance
float g_barcode_old_distance;

// Task handle to notify when to get distance
static TaskHandle_t g_barcode_notify_dist_task;

// (Unit testing) Shared distance buffer between wheel encode 
// and barcode driver
float g_shared_dist_buffer;
SemaphoreHandle_t g_barcode_dist_mutex;

bool g_barcode_array[10];
int8_t g_barcode_array_count;

/* <!> Change time_passed -> distance*/
struct barcode_struct {
    uint64_t last_time; /* Kept solely to determine time_passed */
    uint16_t time_passed; /* Used to determine pulse width */

    // High is white
    bool high;           /* Determine if high or low pulse */
    bool is_short;
};

// Buffer to store barcode info
static struct barcode_struct g_barcode_buffer;

// (Unit testing) TaskHandle to store distance
static TaskHandle_t g_barcode_dist_task;


void barcode_edge_irq(uint gpio, uint32_t events){

    xTaskNotify(g_barcode_notify_dist_task, events, eIncrement);

    if (!(events == (GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE))){

        /* Get current time*/
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        uint16_t old_time_passed = g_barcode_buffer.time_passed;

        /* Calculate time passed based on last time, and store current time as the new last time */
        g_barcode_buffer.time_passed = current_time - g_barcode_buffer.last_time;
        g_barcode_buffer.last_time = current_time;

        /* Record down high or low */
        /* <!> Work on *2 */
        if (old_time_passed == 0) {printf("<Barcode length is an estimate for now!>\n");}
        if (old_time_passed > g_barcode_buffer.time_passed * 2){
                printf("<Barcode length>\tShort\n");
                g_barcode_buffer.is_short = 1;
            }
        else if (old_time_passed * 2 < g_barcode_buffer.time_passed){
                printf("<Barcode length>\tLong\n");
                g_barcode_buffer.is_short = 0;
            }
        else {
            if (g_barcode_buffer.is_short){
                printf("<Barcode length>\tShort\n");
            }
            else{
                printf("<Barcode length>\tLong\n");
            }
        }
        if (events == GPIO_IRQ_EDGE_RISE){
            g_barcode_buffer.high = false;
            printf("<Barcode> Low pulse for %d ms\n", g_barcode_buffer.time_passed);
        }

        /* Exited black region */
        else if (events == GPIO_IRQ_EDGE_FALL){
            g_barcode_buffer.high = true;
            printf("<Barcode> High pulse for %d ms\n", g_barcode_buffer.time_passed);
        }

        /* Push barcode length into array. Short - 1; Long - 0;*/
        g_barcode_array[g_barcode_array_count++] = g_barcode_buffer.is_short;
        if (g_barcode_array_count > 9) {g_barcode_array_count = 0;}

        /* Print barcode array */
        /*
        printf("[");
        for(int loop = 0; loop < 10; loop++){
            printf("%d ,", g_barcode_array[loop]);
        }
        printf("]\n");
        */
    }

    /* <!> Add digital support (debounce and assume long -> short ?)*/
    else {
        printf("<Barcode> Edge rise and fall detected simultaneously");
    }
}

void barcode_distance_task(__unused void *params){
    while (1){
        /**
         * ulTaskNotifyTake notes:
         * For xClearCountOnExit parameter,
         * pdFALSE -> Task notification value is decremented before Take exits
         * pdTRUE -> Task notification value is reset to 0 before Take exits
         * 
         * Returns task notification value before decrement or clear 
         * */
        //From irq: xTaskNotify(g_barcode_notify_dist_task, events, eIncrement);

        // Block here until notified
        uint32_t task_notif_val = 0;
        xTaskNotifyWait(0,0, &task_notif_val, portMAX_DELAY);

        //uint32_t task_notif_val = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        // Block until you get mutex
        xSemaphoreTake(g_barcode_dist_mutex, portMAX_DELAY);
        printf("I am notified of event %d\n", task_notif_val);
        xSemaphoreGive(g_barcode_dist_mutex);
        vTaskDelay(500);
    }

}

void barcode_init() {
    // Configure the button pin as an input
    gpio_init(BARCODE_PIN);
    gpio_set_dir(BARCODE_PIN, GPIO_IN);
    gpio_pull_down(BARCODE_PIN); // Use pull-down resistor

    g_barcode_buffer.last_time = 0;
    g_barcode_buffer.time_passed = 0;
    g_barcode_buffer.high = 0;
    g_barcode_array_count = 0;

    g_barcode_dist_mutex = xSemaphoreCreateMutex();

    gpio_set_irq_enabled_with_callback(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &barcode_edge_irq);
    
    // Create tasks
    //xTaskCreate(barcode_distance_task, "BarcodeDistTask", configMINIMAL_STACK_SIZE, NULL, 8, &g_barcode_notify_dist_task);

    //Should be in main
    //vTaskStartScheduler();

}

/**
 * For testing purposes 
 * To do: Add in #ifdef to make sure this main function is created
 * only when certain parameters are set
 */
int main() {

    stdio_init_all();

    /* Declare in main function */
    barcode_init();
    vTaskStartScheduler();

    /* Unit testing */
    float g_shared_dist_buffer = 0;

    while (true);
}