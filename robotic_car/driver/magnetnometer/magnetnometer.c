#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>

#define I2C_PORT i2c0
#define I2C_CLOCK_FREQ 100000
#define I2C_SDA_PIN 0 // GPIO pin for I2C Serial Data (SDA)
#define I2C_SCL_PIN 1 // GPIO pin for I2C Serial Clock (SCL)

#define ACC_ADDR 0x19 // I2C address of accelerometer
#define MAG_ADDR 0x1E // I2C address of magnetometer

// Register address of accelerometer
#define ACC_CTRL_REG1 0x20 // Control Register 1
#define ACC_X_LSB 0x28 // Output Register X (LSB)
#define ACC_X_MSB 0x29 // Output Register X (MSB)
#define ACC_Y_LSB 0x2A // Output Register Y (LSB)
#define ACC_Y_MSB 0x2B // Output Register Y (MSB)
#define ACC_Z_LSB 0x2C // Output Register Z (LSB)
#define ACC_Z_MSB 0x2D // Output Register Z (MSB)

// Register address of magnetometer
#define MAG_MR_REG 0x02 // Control Register MR
#define MAG_X_LSB 0x04 // Output Register X (LSB)
#define MAG_X_MSB 0x03 // Output Register X (MSB)
#define MAG_Y_LSB 0x08 // Output Register Y (LSB)
#define MAG_Y_MSB 0x07 // Output Register Y (MSB)
#define MAG_Z_LSB 0x06 // Output Register Z (LSB)
#define MAG_Z_MSB 0x05 // Output Register Z (MSB)

typedef struct {
    int16_t x, y, z;
} Acc_Data;

typedef struct {
    int16_t x, y, z;
} Mag_Data;

void configure(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t data[] = {reg, value};
    i2c_write_blocking(I2C_PORT, addr, data, 2, false);
}

uint8_t read(uint8_t addr, uint8_t reg) {
    uint8_t data;
    i2c_write_blocking(I2C_PORT, addr, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, addr, &data, 1, false);
    return data;
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, I2C_CLOCK_FREQ);
    i2c_set_slave_mode(I2C_PORT, false, 0);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    configure(ACC_ADDR, ACC_CTRL_REG1, 0x57);
    configure(MAG_ADDR, MAG_MR_REG, 0x00);
    sleep_ms(3000); // Allow time for sensor initialization

    while (true) {
        // Extract and format the accelerometer data
        Acc_Data acc_data;
        acc_data.x = (int16_t)((read(ACC_ADDR, ACC_X_MSB) << 8) | read(ACC_ADDR, ACC_X_LSB));
        acc_data.y = (int16_t)((read(ACC_ADDR, ACC_Y_MSB) << 8) | read(ACC_ADDR, ACC_Y_LSB));
        acc_data.z = (int16_t)((read(ACC_ADDR, ACC_Z_MSB) << 8) | read(ACC_ADDR, ACC_Z_LSB));

        // Extract and format the magnetometer data
        Mag_Data mag_data;
        mag_data.x = (int16_t)((read(MAG_ADDR, MAG_X_MSB) << 8) | read(MAG_ADDR, MAG_X_LSB));
        mag_data.y = (int16_t)((read(MAG_ADDR, MAG_Y_MSB) << 8) | read(MAG_ADDR, MAG_Y_LSB));
        mag_data.z = (int16_t)((read(MAG_ADDR, MAG_Z_MSB) << 8) | read(MAG_ADDR, MAG_Z_LSB));

        float heading = atan2(mag_data.y, mag_data.x) * 180 / M_PI;

        if (heading < 0)
            heading += 360;

        printf("Accelerometer x: %d y: %d z: %d\n", acc_data.x, acc_data.y, acc_data.z);
        printf("Magnetometer x: %d y: %d z: %d\n", mag_data.x, mag_data.y, mag_data.z);
        printf("Heading: %.2f\n\n", heading);

        sleep_ms(200);
    }

    return 0;
}