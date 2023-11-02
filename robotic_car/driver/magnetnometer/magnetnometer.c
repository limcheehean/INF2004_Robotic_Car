#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Configuration Constants
#define I2C_ID i2c0 // I2C peripheral ID
#define CLOCK_FREQUENCY 100000 // Clock frequency for I2C communication
#define I2C_SDA_PIN 0 // GPIO pin for I2C SDA
#define I2C_SCL_PIN 1 // GPIO pin for I2C SCL
#define MAG_ADDR 0x1E // I2C address of the magnetometer

// Magnetometer Data Structure
typedef struct {
    int16_t x; // X-axis data
    int16_t y; // Y-axis data
    int16_t z; // Z-axis data
} MagnetometerData;

int main() {
    // Initialize standard I/O
    stdio_init_all();
    
    // Initialize I2C and configure GPIO pins for I2C communication
    i2c_init(I2C_ID, CLOCK_FREQUENCY);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // Configure the magnetometer for continuous conversion mode
    uint8_t config[] = {0x00}; // Mode selection (continuous-conversion)
    i2c_write_blocking(I2C_ID, MAG_ADDR, config, sizeof(config), false);
    sleep_ms(3000); // Delay to allow the magnetometer to stabilize

    while (true) {
        // Read magnetometer data
        uint8_t data[6];
        i2c_read_blocking(I2C_ID, MAG_ADDR, data, sizeof(data), false);

        // Extract and format the magnetometer data
        MagnetometerData magnetometerData;
        magnetometerData.x = (int16_t)((data[1] << 8) | data[0]) >> 4;
        magnetometerData.y = (int16_t)((data[3] << 8) | data[2]) >> 4;
        magnetometerData.z = (int16_t)((data[5] << 8) | data[4]) >> 4;

        // Print the magnetometer data
        printf("Magnetometer Data - X: %d, Y: %d, Z: %d\n", magnetometerData.x, magnetometerData.y, magnetometerData.z);

        sleep_ms(1000); // Delay between successive readings
    }

    return 0;
}