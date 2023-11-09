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

#define BARCODE_PIN 9

uint32_t last_press_time = 0;  // Store the last time the button was pressed

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

void barcode_edge_irq(uint gpio, uint32_t events){

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
    else {
        printf("<Barcode> Edge rise and fall detected simultaneously");
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

    gpio_set_irq_enabled_with_callback(BARCODE_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &barcode_edge_irq);

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


    while (true);
}