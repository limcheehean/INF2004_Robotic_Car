#ifdef OLD_BUFFER_TESTING

#include "pico/stdlib.h"
#include <stdio.h>

// Define the size of the barcode buffer
#define BARCODE_BUFFER_SIZE 9

// Structure to represent the barcode buffer
typedef struct
{
    uint16_t buffer; // Binary representation
} BarcodeBuffer_t;

// Function to initialize the barcode buffer
void init_barcode_buffer(BarcodeBuffer_t *barcode_buffer)
{
    barcode_buffer->buffer = 0x0;
}

// Function to retrieve a specific bit from the barcode buffer
bool barcode_buffer_get(BarcodeBuffer_t *barcode_buffer, int logical_index)
{
    // Ensure the logical_index is within bounds
    if (logical_index >= BARCODE_BUFFER_SIZE)
    {
        logical_index = BARCODE_BUFFER_SIZE - 1;
    }

    // Copy the buffer and retrieve the specified bit
    uint16_t buffer_copy = barcode_buffer->buffer;
    return (buffer_copy >> logical_index) & 0x1;
}

// Function to insert data into the barcode buffer
void barcode_buffer_put(BarcodeBuffer_t *barcode_buffer, bool data_to_insert)
{
    barcode_buffer->buffer = (barcode_buffer->buffer << 1) + data_to_insert;
}

// Function to reverse the bits in the barcode buffer
uint16_t barcode_buffer_reverse(BarcodeBuffer_t *barcode_buffer)
{
    uint16_t reversed_buffer = 0;
    uint16_t buffer = barcode_buffer->buffer;

    for (unsigned int i = 0; i < BARCODE_BUFFER_SIZE; i++)
    {
        // For every bit in buffer, if bit is 1, set binary values from the back
        if (buffer & (1 << i))
        {
            reversed_buffer |= (1 << ((BARCODE_BUFFER_SIZE - 1) - i));
        }
    }

    return reversed_buffer;
}

// Function to clear the barcode buffer
void barcode_buffer_clear(BarcodeBuffer_t *barcode_buffer)
{
    barcode_buffer->buffer = 0x0;
}

#endif // OLD_BUFFER_TESTING
