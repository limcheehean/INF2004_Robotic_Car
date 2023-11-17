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
#include "FreeRTOS.h"

#define BARCODE_PIN 9

#define BARCODE_BUFFER_SIZE 10
#define BARCODE_BUFFER_READ_OFFSET (BARCODE_BUFFER_SIZE - 9)
//#define BARCODE_MODULE_LOCAL


// Task handle to notify when to get distance

// Data structure to store barcode data collected from ISR
typedef struct {
    uint64_t last_time; /* Kept solely to determine time_passed */
    uint64_t time_passed; /* Used to determine pulse width */
    uint64_t current_time;

    // High is white
    bool high;           /* Determine if high or low pulse */
    bool is_short;
} BarcodeISRData;

typedef struct {
    int buffer_curr_index;
    bool array[BARCODE_BUFFER_SIZE];
} BarcodeBuffer;

typedef struct{
    TaskHandle_t barcode_interpret_task;
    bool is_reading;
    BarcodeISRData barcode_isr_data;
    BarcodeBuffer barcode_buffer;
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

void init_barcode_buffer(BarcodeModule * barcode_module, int buffer_size){
    barcode_module->barcode_buffer.buffer_curr_index = BARCODE_BUFFER_SIZE - 1;
}

bool barcode_buffer_get(int logical_index){
    //BarcodeBuffer barcode_buffer = barcode_module->barcode_buffer;
    BarcodeBuffer * barcode_buffer = & (get_barcode_module() -> barcode_buffer);
    /***
     * Barcode buffer is a circular buffer
     * [1,0,1,0,1]
     *    ^
     * Current index: 1
     * Target logical index: 2
     * Target actual index: 4
     * = current index + logical index + 1
     * [1,0,1,0,1]
     *        ^
     * Current index: 4
     * Target logical index: 3
     * Target actual index: 2
     * = (current index + logical index + 1) - ARRAYSIZE - 1
     * = (8) - 5 - 1 = 2
     * */ 
    int index = barcode_buffer->buffer_curr_index + logical_index + 1;
    if (index >= BARCODE_BUFFER_SIZE){
        index -= BARCODE_BUFFER_SIZE;
    }
    //printf("\n RETURN %d\n", barcode_buffer->array[index]);
    //printf("Reading at index %d\n", index);
    return barcode_buffer->array[index];
}

void barcode_buffer_put(bool data_to_insert){

    BarcodeBuffer * barcode_buffer = & (get_barcode_module() -> barcode_buffer);
    /***
     * Barcode buffer is a circular buffer
     * [1,0,1,0,1]
     *  ^ current index at 0, has newest value
     * If I want to insert a new value, 
     * increment current index and insert at current index
     * [1,0,1,0,1]
     *          ^ current index 4, has newest value
     * To insert when logical array is full,
     * reset current index to 0 and insert at current index
     * */ 
    if (++barcode_buffer->buffer_curr_index >= BARCODE_BUFFER_SIZE){
        barcode_buffer->buffer_curr_index = 0;
    } 
    //printf("Insert at index %d\n", barcode_buffer->buffer_curr_index);
    barcode_buffer->array[barcode_buffer->buffer_curr_index] = data_to_insert;
    //printf("\n INSERTED %d \n", barcode_buffer->array[barcode_buffer->buffer_curr_index]);

}

void barcode_buffer_clear(){
    BarcodeBuffer * barcode_buffer = & (get_barcode_module() -> barcode_buffer);
    for (int i = 0; i < BARCODE_BUFFER_SIZE; i++){
        barcode_buffer->array[i] = 0;
    }
}

void interpret_barcode(){
    BarcodeModule * bm = get_barcode_module();
    int index_to_copy = 0;
    bool is_start_stop = 1;
    static int start_stop_barcode_fwd[] = {1,0,1,1,0,1,0,1,1};
    static int start_stop_barcode_bwd[] = {1,1,0,1,0,1,1,0,1};

    static int char_A[] =                  {0,1,1,1,1,0,1,1,0};

    bool is_A = 1;

    if (!bm->is_reading){
        /* To do: Quiet Zone should be used to determine short length*/
        for (int i = BARCODE_BUFFER_READ_OFFSET; i < 10; i++){
            
            if (barcode_buffer_get(i) != start_stop_barcode_fwd[i - BARCODE_BUFFER_READ_OFFSET]){
                is_start_stop = 0;
                break;
            }
        }
        if (is_start_stop){
            printf("\n<BARCODE START>\n\n");
            barcode_buffer_clear();
            bm -> is_reading = 1;
        }
    }
    else {
        
        for (int i = BARCODE_BUFFER_READ_OFFSET; i < 10; i++){
            if (is_A){
                if (barcode_buffer_get(i) != char_A[i - BARCODE_BUFFER_READ_OFFSET]){
                    is_A = 0;
                    //break;
                }
            }
            if (is_start_stop){
                if (barcode_buffer_get(i) != start_stop_barcode_fwd[i - BARCODE_BUFFER_READ_OFFSET]){
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
            barcode_buffer_clear();
        }
        else if (is_start_stop){
            printf("\n<BARCODE START>\n\n");
            barcode_buffer_clear();
            bm -> is_reading = 0;
        }
    }

}

void barcode_edge_irq(uint gpio, uint32_t events){
    //printf("g_shared_dist_buffer is %2.2f\n", g_shared_dist_buffer);

    BarcodeModule * bm = get_barcode_module();
    /* Get current time*/
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (!(events == (GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE))){
        BarcodeISRData * barcode_isr_data = &(bm->barcode_isr_data);
        uint64_t old_time_passed = bm->barcode_isr_data.time_passed;

        /* Calculate time passed based on last time, and store current time as the new last time */
        barcode_isr_data->time_passed = current_time - barcode_isr_data->last_time;
        barcode_isr_data->last_time = current_time;
        barcode_isr_data->current_time = current_time;

        /* Record down high or low */
        if (old_time_passed == 0) {printf("<Barcode length is an estimate for now!>\n");}
        if (old_time_passed > barcode_isr_data->time_passed * 2){
                //printf("<Barcode length>\tShort\n");
                barcode_isr_data->is_short = 1;
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
            /* Was white line */
            barcode_isr_data->high = false;
            printf("<Barcode> |%d\t | Low\t| %d ms\t|\n",bm->barcode_isr_data.is_short, bm->barcode_isr_data.time_passed);
        }

        /* Exited black region */
        else if (events == GPIO_IRQ_EDGE_FALL){
            /* Was black line */
            barcode_isr_data->high = true;
            printf("<Barcode> |%d\t |High\t| %d ms\t| %2.2f\t|\n",bm->barcode_isr_data.is_short, bm->barcode_isr_data.time_passed);
        }

        /* Push barcode length into array. Short - 1; Long - 0;*/
        //bm->barcode_array[bm->barcode_array_index++] = bm->barcode_isr_data.is_short;
        barcode_buffer_put(bm->barcode_isr_data.is_short);

        printf("[");
        
        for(int loop = 0; loop < 10; loop++){
            printf("%d,", barcode_buffer_get(loop));
        }
        printf("]\n");
        interpret_barcode();
        
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
    barcode_module -> is_reading = false;

    init_barcode_buffer(barcode_module, BARCODE_BUFFER_SIZE);

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