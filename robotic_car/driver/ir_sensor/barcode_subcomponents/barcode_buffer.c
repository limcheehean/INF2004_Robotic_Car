
#ifdef OLD_BUFFER_TESTING
#include "pico/stdlib.h"
#include <stdio.h>

#define BARCODE_BUFFER_SIZE 9

typedef struct {
    uint16_t buffer; //binary
} BarcodeBuffer_t;


void init_barcode_buffer(BarcodeBuffer_t * barcode_buffer){

    barcode_buffer ->  buffer = 0x0;
}

bool barcode_buffer_get(BarcodeBuffer_t * barcode_buffer, int logical_index){

    if (!logical_index < BARCODE_BUFFER_SIZE){
        logical_index = BARCODE_BUFFER_SIZE - 1;
    }

    uint16_t buffer_copy = barcode_buffer -> buffer;
    return (buffer_copy >> logical_index) & 0x1;


}

void barcode_buffer_put(BarcodeBuffer_t * barcode_buffer, bool data_to_insert){

    barcode_buffer -> buffer = (barcode_buffer->buffer << 1) + data_to_insert;

}

uint16_t barcode_buffer_reverse(BarcodeBuffer_t * barcode_buffer){

    uint16_t reversed_buffer = 0;
    uint16_t buffer = barcode_buffer->buffer;

    for (unsigned int i = 0; i < BARCODE_BUFFER_SIZE; i++) {

        /* For every bit in buffer, if bit is 1 */
        if (buffer & (1 << i)) {
            /* Use bitwise or to set binary values from the back */
            reversed_buffer |= (1 << ((BARCODE_BUFFER_SIZE - 1) - i));
        }
    }

    return reversed_buffer;
}

void barcode_buffer_clear(BarcodeBuffer_t * barcode_buffer){
    barcode_buffer = 0x0;
}
#endif
