#ifndef BARCODE_ISR_DATA_H_
#define BARCODE_ISR_DATA_H_
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include "queue.h"
#define BARCODE_BUFFER_SIZE 10

/*

Possible improvements:
Refer to 
https://www.freertos.org/fr-content-src/uploads/2018/07/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf

Nice-to-haves;
 - Variable queue message length (Possible implementation: task 1 or interrupt mallocs() -> put pointer into queue -> task2 free() space)
 - Multi-core support (allows more complex sensor fusion)

*/

/* Used for communication between Interrupt and Task */
typedef struct {
    //uint64_t last_time; /* Kept solely to determine time_passed */
    uint64_t time_passed; /* Used to determine pulse width */
    float wheel_encoder_speed;
    //float wheel_encoder_time;
    uint64_t current_time;

    // High is white
    bool high;           /* Determine if high or low pulse */
    bool is_short;
} BarcodeISRData_t;

typedef struct {
    int buffer_curr_index;
    bool array[BARCODE_BUFFER_SIZE];
} BarcodeBuffer_t;

extern QueueHandle_t g_barcode_interpret_queue;

#endif  // BARCODE_ISR_DATA_H_