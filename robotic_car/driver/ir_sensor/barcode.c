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
#include "pico/stdlib.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define BARCODE_PIN 9

//#define BARCODE_MODULE_LOCAL


// Task handle to notify when to get distance

// Data structure to store barcode data collected from ISR
typedef struct {
    uint64_t last_time; /* Kept solely to determine time_passed */
    uint16_t time_passed; /* Used to determine pulse width */
    uint64_t current_time;

    // High is white
    bool high;           /* Determine if high or low pulse */
    bool is_short;
} BarcodeISRData;

typedef struct{
    TaskHandle_t barcode_interpret_task;
    bool barcode_array[10];
    uint16_t barcode_array_count;
    BarcodeISRData barcode_isr_data;
} BarcodeModule;


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
/*
BarcodeISRData * get_barcode_isr_data(){
    static BarcodeISRData barcode_isr_data;
    return &barcode_isr_data;
}*/
#endif

void barcode_edge_irq(uint gpio, uint32_t events){
    //printf("g_shared_dist_buffer is %2.2f\n", g_shared_dist_buffer);

    BarcodeModule * bm = get_barcode_module();
    /* Get current time*/
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (!(events == (GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE))){
        BarcodeISRData * barcode_isr_data = &(bm->barcode_isr_data);
        uint16_t old_time_passed = bm->barcode_isr_data.time_passed;

        /* Calculate time passed based on last time, and store current time as the new last time */
        barcode_isr_data->time_passed = current_time - barcode_isr_data->last_time;
        barcode_isr_data->last_time = current_time;
        barcode_isr_data->current_time = current_time;

        /* Record down high or low */
        if (old_time_passed == 0) {printf("<Barcode length is an estimate for now!>\n");}
        if (old_time_passed > barcode_isr_data->time_passed * 2){
                //printf("<Barcode length>\tShort\n");
                bm->barcode_isr_data.is_short = 1;
            }
        else if (old_time_passed * 2 < barcode_isr_data->time_passed){
                //printf("<Barcode length>\tLong\n");
                barcode_isr_data->is_short = 0;
            }
        else {
            if (barcode_isr_data->is_short){
                //printf("<Barcode length>\tShort\n");
            }
            else{
                //printf("<Barcode length>\tLong\n");
            }
        }
        if (events == GPIO_IRQ_EDGE_RISE){
            barcode_isr_data->high = false;
            printf("<Barcode> |%d\t | Low\t| %d ms\t|\n",bm->barcode_isr_data.is_short, bm->barcode_isr_data.time_passed);
        }

        /* Exited black region */
        else if (events == GPIO_IRQ_EDGE_FALL){
            barcode_isr_data->high = true;
            printf("<Barcode> |%d\t |High\t| %d ms\t| %2.2f\t|\n",bm->barcode_isr_data.is_short, bm->barcode_isr_data.time_passed);
        }

        /* Push barcode length into array. Short - 1; Long - 0;*/
        bm->barcode_array[bm->barcode_array_count++] = bm->barcode_isr_data.is_short;
        printf("Pushed data in barcode, count is %d, address is %p\n", bm->barcode_array_count, &(bm->barcode_array_count));
        if (bm->barcode_array_count > 9) {
            printf("Printing array count...\n");
            /* Print barcode array */
            printf("[");
            for(int loop = 0; loop < 10; loop++){
                printf("%d ,", bm->barcode_array[loop]);
            }
            printf("]\n");
            bm->barcode_array_count = 0;
        }
    }

    /* <!> Add digital support (debounce and assume long -> short ?)*/
    /* Debounce or not, this may occur if another interrupt was hogging up processor time*/
    /** 
     * Its' possible to ignore R&F, but additional logic is needed to ensure that
     * after a RISE, a FALL occurs, and vice versa
     * */
    else {
        //printf("<Barcode> Edge rise and fall detected simultaneously\n");
        printf("R&F\n");
    }
}

void barcode_module_init() {
    // Configure the button pin as an input
    gpio_init(BARCODE_PIN);
    gpio_set_dir(BARCODE_PIN, GPIO_IN);
    gpio_pull_down(BARCODE_PIN); // Use pull-down resistor
    
    BarcodeModule * barcode_module = get_barcode_module();
    BarcodeISRData * barcode_isr_data = &(barcode_module->barcode_isr_data);
    barcode_isr_data->last_time = 0;
    barcode_isr_data->time_passed = 0;
    barcode_isr_data->high = 0;
    barcode_module -> barcode_array_count = 0;

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
    float b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.last_time) / 1000000.0f);
    current_time = current_time;

    current_time = time_us_64();
    //g_shared_dist_buffer -= 1;
    a = (float)g_shared_dist_buffer / 40 * 33.2f;
    b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.last_time) / 1000000.0f);
    current_time = current_time;

    current_time = time_us_64();
    //g_shared_dist_buffer += 1;
    a = (float)g_shared_dist_buffer / 40 * 33.2f;
    b = 33.2f / 40 / ((float)(current_time - get_barcode_module()->barcode_isr_data.last_time) / 1000000.0f);
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
int main() {

    stdio_init_all();

    /* Declare in main function */
    // Instantiate barcode module
    //BarcodeModule bm; /* Create in stack */
    //barcode_class_init(bm); /* Store address in memory */

    barcode_module_init();
    //vTaskStartScheduler();

    /* Unit testing */
    float g_shared_dist_buffer = 13212.231;
    gpio_init(27);
    gpio_set_dir(27, GPIO_IN);
    gpio_pull_up(27);
    gpio_set_irq_enabled_with_callback(27, GPIO_IRQ_LEVEL_LOW /* Must not start with 27 connected to gnd*/, true, &generic_irq);

    

    while (true);
}