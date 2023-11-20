
#include "pico/stdlib.h"
#include <stdio.h>

#define BARCODE_BUFFER_SIZE 10
#define BARCODE_BUFFER_READ_OFFSET (BARCODE_BUFFER_SIZE - 9)

typedef struct {
    int buffer_curr_index;
    bool array[BARCODE_BUFFER_SIZE];
} BarcodeBuffer_t;


void init_barcode_buffer(BarcodeBuffer_t * barcode_buffer){
    barcode_buffer -> buffer_curr_index = BARCODE_BUFFER_SIZE - 1;
}

bool barcode_buffer_get(BarcodeBuffer_t * barcode_buffer, int logical_index){
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

void barcode_buffer_put(BarcodeBuffer_t * barcode_buffer, bool data_to_insert){

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

void barcode_buffer_clear(BarcodeBuffer_t * barcode_buffer){
    for (int i = 0; i < BARCODE_BUFFER_SIZE; i++){
        barcode_buffer->array[i] = 0;
    }
}
