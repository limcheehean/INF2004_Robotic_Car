#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"

/* Used for communication between Interrupt and Task */
typedef struct {
    //uint64_t last_time; /* Kept solely to determine time_passed */
    uint64_t time_passed; /* Used to determine pulse width */
    uint64_t current_time;

    // High is white
    bool high;           /* Determine if high or low pulse */
    bool is_short;
} BarcodeISRData_t;

QueueHandle_t g_barcode_interpret_queue;
