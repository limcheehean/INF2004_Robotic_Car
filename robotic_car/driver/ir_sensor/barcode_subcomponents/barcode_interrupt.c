/**
 * Interrupt-based ir line sensor
 * Usage instruction:
 *  - Connect BARCODE_PIN (Pin 9) to Analogue Pin of IR Line Sensor (or Digital Pin, with minimum sensitivity)
 *      Note that Digital pin may trigger interrupt of EDGE RISE and EDGE LOW at same time (event mask will include both high and low in 1 interrupt)
 *      Each interrupt from Analogue pin will only be EDGE RISE or EDGE LOW, not both at same time
 *      This is needed if the car moves too fast.
 * - Debounce is not required, tested with no issues so far
 * */

/**
 * Note: Analogue pin seens to interrupt only occur at at maximum level
 * 
 * To do:
 *  - Read barcode
 *  - Read barcode in reverse
 * */
//#include "barcode_buffer.c"
#include "barcode_isr_data.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#ifndef INCLUDE_WHEEL_ENCODER
#include "../../encoder/wheel_encoder.h"
#endif


#define BARCODE_PIN 9

//#define BARCODE_MODULE_LOCAL


/*********
 * Structs
 * *******/

typedef struct{
    TaskHandle_t barcode_interpret_task;
    bool is_reading;
    BarcodeISRData_t barcode_isr_data;
} BarcodeModule;

/******************
 * Global variables
 * 
 * ****************/

int unit_test_var = 5;

#ifdef BARCODE_MODULE_LOCAL

/* Not working because barcode_module is too huge, the register will run out of space*/
/* Allow BarcodeModule to be instantiated as a local variable in main function (use register) */
static BarcodeModule * g_barcode_module;

BarcodeModule * barcode_class_init(BarcodeModule bc){
    g_barcode_module = &bc;
    return g_barcode_module;
}
BarcodeModule * get_barcode_module() {
    return g_barcode_module;
}
#else

/* Instantiate BarcodeModule as static variable */
BarcodeModule * get_barcode_module() {
    static BarcodeModule barcode_module;
    return &barcode_module;
}
#endif

void init_barcode_intr_queue(QueueHandle_t * barcode_interpret_queue){
    g_barcode_interpret_queue =  *barcode_interpret_queue;
}

void barcode_edge_irq(uint gpio, uint32_t events){

    //printf("g_shared_dist_buffer is %2.2f\n", g_shared_dist_buffer);
    
    BarcodeModule * bm = get_barcode_module();
    static uint64_t last_time = 0;

    /* Get current time*/
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (!(events == (GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE))){

        BarcodeISRData_t * barcode_isr_data = &(bm->barcode_isr_data);

        barcode_isr_data->time_passed = current_time - barcode_isr_data->current_time;

        barcode_isr_data->current_time = current_time;

        if (events == GPIO_IRQ_EDGE_RISE){

            barcode_isr_data->high = false;

        }
    

        /* Exited black region */
        else if (events == GPIO_IRQ_EDGE_FALL){
            
            barcode_isr_data->high = true;

        }

        //interpret_barcode();
        barcode_isr_data -> wheel_encoder_speed = get_encoder_data()->left_encoder.current_speed;
        //barcode_isr_data -> wheel_encoder_time = get_encoder_data()->left_encoder.last_time;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (!xQueueSendFromISR(g_barcode_interpret_queue, &bm->barcode_isr_data /*barcode_isr_data*/, &xHigherPriorityTaskWoken)){
            //printf("\n\nBuffer full\n\n");
        };

        
        
    }

    /* <!> Add digital support (debounce and assume long -> short ?)*/
    /* Debounce or not, this may occur if another interrupt was hogging up processor time*/
    /** 
     * Its' possible to ignore R&F, but additional logic is needed to ensure that
     * after a RISE, a FALL occurs, and vice versa
     * */
    else {
        //printf("<Barcode> Edge rise and fall detected simultaneously\n");
        //printf("R&F\n");
    }
}

void barcode_interrupt_init() {
    // Configure the button pin as an input
    gpio_init(BARCODE_PIN);
    gpio_set_dir(BARCODE_PIN, GPIO_IN);
    gpio_pull_down(BARCODE_PIN); // Use pull-down resistor
    
    BarcodeModule * barcode_module = get_barcode_module();
    //BarcodeBuffer_t * barcode_buffer = get_barcode_buffer();
    BarcodeISRData_t * barcode_isr_data = &(barcode_module->barcode_isr_data);
    barcode_isr_data->current_time = 0;
    barcode_isr_data->time_passed = 0;
    barcode_isr_data->high = 0;
    barcode_module -> is_reading = false;

    //init_barcode_buffer(barcode_buffer, BARCODE_BUFFER_SIZE);

    gpio_set_irq_enabled_with_callback(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &barcode_edge_irq);
    
    // Create tasks
    //xTaskCreate(barcode_distance_task, "BarcodeDistTask", configMINIMAL_STACK_SIZE, NULL, 8, &g_barcode_notify_dist_task);

    //Should be in main
    //vTaskStartScheduler();

}

/***
 * Unit testing functions
 * 
 * */
// (Unit testing) TaskHandle to store distance
static TaskHandle_t g_barcode_dist_task;

// (Unit testing) Distance Incrementing
void dist_increm_irq(uint gpio, uint32_t events){
    
    /* Simulates wheel encode isr's calculation */
    /* Increased computation load by 3 for testing purposes */ 
    uint64_t current_time = time_us_64();
    float g_shared_dist_buffer = 1;
    //g_shared_dist_buffer += 1;
    float a = (float)g_shared_dist_buffer / 40 * 33.2f;
    float b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.current_time) / 1000000.0f);
    current_time = current_time;

    current_time = time_us_64();
    //g_shared_dist_buffer -= 1;
    a = (float)g_shared_dist_buffer / 40 * 33.2f;
    b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.current_time) / 1000000.0f);
    current_time = current_time;

    current_time = time_us_64();
    //g_shared_dist_buffer += 1;
    a = (float)g_shared_dist_buffer / 40 * 33.2f;
    b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.current_time) / 1000000.0f);
    current_time = current_time;
}

void generic_irq(uint gpio, uint32_t events){
    if (gpio == 27){
        dist_increm_irq(gpio, events);
    }
    else if (gpio == 9){
        barcode_edge_irq(gpio, events);
    }
}

/**
 * For testing purposes 
 * To do: Add in #ifdef to make sure this main function is created
 * only when certain parameters are set
 */

#ifdef BARCODE_INTERRUPT_TEST
int main() {
    #include "barcode_interpret.c"

    stdio_init_all();

    /* Declare in main function */
    // Instantiate barcode module
    //BarcodeModule bm; /* Create in stack */
    //barcode_class_init(bm); /* Store address in memory */

    barcode_interrupt_init();


    /* Unit testing */
    float g_shared_dist_buffer = 13212.231;
    gpio_init(27);
    gpio_set_dir(27, GPIO_IN);
    gpio_pull_up(27);
    gpio_set_irq_enabled_with_callback(27, GPIO_IRQ_LEVEL_LOW /* Must not start with 27 connected to gnd*/, true, &generic_irq);

    

    while (true);
}
#endif