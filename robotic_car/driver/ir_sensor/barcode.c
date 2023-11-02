#include "pico/stdlib.h"
#include <stdio.h>

#define BARCODE_PIN 9
#define DEBOUNCE_TIME_MS 50  // Define a debounce time of 50 milliseconds

uint32_t last_press_time = 0;  // Store the last time the button was pressed


static bool g_barcode_debouncing = false;

bool g_barcode_array[10];
int8_t g_barcode_array_count;

struct barcode_struct {
    uint64_t last_time; /* Kept solely to determine time_passed */
    uint16_t time_passed; /* Used to determine pulse width */
    bool high;           /* Determine if high or low pulse */
};

struct barcode_distance_struct {
    uint64_t last_time; /* Kept solely to determine time_passed */
    uint16_t time_passed; /* Used to determine pulse width */
    bool is_short;           /* Determine if high or low pulse */
};
// Buffer to store barcode info
static struct barcode_struct g_barcode_buffer;

static struct barcode_distance_struct g_barcode_low_buffer;

int64_t debounce_isr(alarm_id_t id, void *user_data) {
    /* Not debouncing anymore */
    g_barcode_debouncing = false;
    return 0;
}

void start_barcode_debounce_alarm(){
    g_barcode_debouncing = true;
    add_alarm_in_ms(-DEBOUNCE_TIME_MS, debounce_isr, NULL, false);
}

void barcode_edge_irq(uint gpio, uint32_t events){

    if (!(events == (GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE))){
        /* Start debounce */
        start_barcode_debounce_alarm();

        /* Get current time*/
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        uint16_t old_time_passed = g_barcode_buffer.time_passed;

        /* Calculate time passed based on last time, and store current time as the new last time */
        g_barcode_buffer.time_passed = current_time - g_barcode_buffer.last_time;
        g_barcode_buffer.last_time = current_time;
        /* Record down high or low */
        if (events == GPIO_IRQ_EDGE_RISE){
            g_barcode_buffer.high = true;
            printf("<Barcode> Low pulse for %d ms\n", g_barcode_buffer.time_passed);
        }

        /* Exited black region */
        else if (events == GPIO_IRQ_EDGE_FALL){
            g_barcode_buffer.high = false;
            printf("<Barcode> High pulse for %d ms\n", g_barcode_buffer.time_passed);

            /* Compare barcode length, determine long or short */
            if (g_barcode_low_buffer.time_passed == 0) {printf("<Barcode length is an estimate for now!>\n");}
            if (g_barcode_low_buffer.time_passed > g_barcode_buffer.time_passed * 2){
                printf("<Barcode length>\tShort\n");
                g_barcode_low_buffer.is_short = 1;
            }
            else if (g_barcode_low_buffer.time_passed * 2 < g_barcode_buffer.time_passed){
                printf("<Barcode length>\tLong\n");
                g_barcode_low_buffer.is_short = 0;
            }
            else {
                if (g_barcode_low_buffer.is_short){
                    printf("<Barcode length>\tShort\n");
                }
                else{
                    printf("<Barcode length>\tLong\n");
                }
            }

            /* Push barcode length into array. Short - 1; Long - 0;*/
            g_barcode_array[g_barcode_array_count++] = g_barcode_low_buffer.is_short;
            if (g_barcode_array_count > 9) {g_barcode_array_count = 0;}

            /* Print barcode array */
            printf("[");
            for(int loop = 0; loop < 10; loop++){
                printf("%d ,", g_barcode_array[loop]);
            }
            printf("]\n");

            /* Store past data */
            g_barcode_low_buffer.time_passed = g_barcode_buffer.time_passed;
            g_barcode_low_buffer.last_time = g_barcode_buffer.last_time;
        }
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

    g_barcode_debouncing = false;
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