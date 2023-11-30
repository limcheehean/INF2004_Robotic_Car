#ifndef MAGNETOMETER
#define MAGNETOMETER
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "../encoder/wheel_encoder.h"

#define I2C_PORT i2c1
#define I2C_CLOCK_FREQ 100000
#define I2C_SDA_PIN 26 // GPIO pin for I2C Serial Data (SDA)
#define I2C_SCL_PIN 27 // GPIO pin for I2C Serial Clock (SCL)

#define ACC_ADDR 0x19 // I2C address of accelerometer
#define MAG_ADDR 0x1E // I2C address of magnetometer

// Register address of accelerometer
#define ACC_CTRL_REG1 0x20 // Control Register 1
#define ACC_X_LSB 0x28     // Output Register X (LSB)
#define ACC_X_MSB 0x29     // Output Register X (MSB)
#define ACC_Y_LSB 0x2A     // Output Register Y (LSB)
#define ACC_Y_MSB 0x2B     // Output Register Y (MSB)
#define ACC_Z_LSB 0x2C     // Output Register Z (LSB)
#define ACC_Z_MSB 0x2D     // Output Register Z (MSB)

// Register address of magnetometer
#define MAG_MR_REG 0x02 // Control Register MR
#define MAG_X_LSB 0x04  // Output Register X (LSB)
#define MAG_X_MSB 0x03  // Output Register X (MSB)
#define MAG_Y_LSB 0x08  // Output Register Y (LSB)
#define MAG_Y_MSB 0x07  // Output Register Y (MSB)
#define MAG_Z_LSB 0x06  // Output Register Z (LSB)
#define MAG_Z_MSB 0x05  // Output Register Z (MSB)

typedef struct
{
    int16_t x, y, z;
} Acc_Data;

typedef struct
{
    int16_t x, y, z;
} Mag_Data;

typedef struct
{
    float x, y, z;
} Calibrated_Data;

int16_t x_max = 0, x_min = 0, y_max = 0, y_min = 0, z_max = 0, z_min = 0;

TaskHandle_t g_magnetometer_task_handle;

QueueHandle_t g_maze_message_queue;
QueueHandle_t g_magnetometer_message_queue;

// Function to configure the I2C communication
void configure(uint8_t addr, uint8_t reg, uint8_t value)
{
    uint8_t data[] = {reg, value};
    i2c_write_blocking(I2C_PORT, addr, data, 2, false);
}

// Function to read a value from a specific register using I2C
uint8_t read(uint8_t addr, uint8_t reg)
{
    uint8_t data;
    i2c_write_blocking(I2C_PORT, addr, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, addr, &data, 1, false);
    return data;
}

// Function to calibrate magnetometer data
Calibrated_Data calibrate(int16_t x, int16_t y, int16_t z)
{
    if (x > x_max)
        x_max = x;

    if (x < x_min)
        x_min = x;

    if (y > y_max)
        y_max = y;

    if (y < y_min)
        y_min = y;

    if (z > z_max)
        z_max = z;

    if (z < z_min)
        z_min = z;

    // Calculate offsets
    float x_offset = (x_max + x_min) / 2;
    float y_offset = (y_max + y_min) / 2;
    float z_offset = (z_max + z_min) / 2;

    // Create a calibrated data structure
    Calibrated_Data calibrated_data;
    calibrated_data.x = x - x_offset;
    calibrated_data.y = y - y_offset;
    calibrated_data.z = z - z_offset;

    return calibrated_data;
}

// Function to configure the magnetometer
int configure_magnetometer()
{
    i2c_init(I2C_PORT, I2C_CLOCK_FREQ);
    i2c_set_slave_mode(I2C_PORT, false, 0);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    configure(ACC_ADDR, ACC_CTRL_REG1, 0x57);
    configure(MAG_ADDR, MAG_MR_REG, 0x00);

    sleep_ms(3000);
}

// Function to get the heading using accelerometer and magnetometer data
float get_heading()
{
    Acc_Data acc_data;
    acc_data.x = (int16_t)((read(ACC_ADDR, ACC_X_MSB) << 8) | read(ACC_ADDR, ACC_X_LSB));
    acc_data.y = (int16_t)((read(ACC_ADDR, ACC_Y_MSB) << 8) | read(ACC_ADDR, ACC_Y_LSB));
    acc_data.z = (int16_t)((read(ACC_ADDR, ACC_Z_MSB) << 8) | read(ACC_ADDR, ACC_Z_LSB));

    // Extract and format the magnetometer data
    Mag_Data mag_data;
    mag_data.x = (int16_t)((read(MAG_ADDR, MAG_X_MSB) << 8) | read(MAG_ADDR, MAG_X_LSB));
    mag_data.y = (int16_t)((read(MAG_ADDR, MAG_Y_MSB) << 8) | read(MAG_ADDR, MAG_Y_LSB));
    mag_data.z = (int16_t)((read(MAG_ADDR, MAG_Z_MSB) << 8) | read(MAG_ADDR, MAG_Z_LSB));

    Calibrated_Data calibrated_data = calibrate(mag_data.x, mag_data.y, mag_data.z);

    float heading = atan2(calibrated_data.y, calibrated_data.x) * 180 / M_PI;
    return heading;
}

// Test function for magnetometer
#ifdef TEST_MAGNETOMETER
int main()
{
    stdio_init_all();
    i2c_init(I2C_PORT, I2C_CLOCK_FREQ);
    i2c_set_slave_mode(I2C_PORT, false, 0);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    configure(ACC_ADDR, ACC_CTRL_REG1, 0x57);
    configure(MAG_ADDR, MAG_MR_REG, 0x00);
    sleep_ms(3000); // Allow time for sensor initialization

    while (true)
    {
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

        Calibrated_Data calibrated_data = calibrate(mag_data.x, mag_data.y, mag_data.z);

        float heading = atan2(calibrated_data.y, calibrated_data.x) * 180 / M_PI;

        if (heading < 0)
            heading += 360;

        printf("Accelerometer x: %d y: %d z: %d\n", acc_data.x, acc_data.y, acc_data.z);
        printf("Magnetometer x: %d y: %d z: %d\n", mag_data.x, mag_data.y, mag_data.z);
        printf("Heading: %.2f\n\n", heading);

        sleep_ms(200);
    }

    return 0;
}
#endif
void magnetometer_task(void *pvParameters)
{

    printf("Magnetometer ready!\n");
    int message;
    bool checking = false;

    QueueHandle_t encoder_mq = get_encoder_data()->message_queue;
    int encoder_message_holder = 0;
    while (true)
    {
        if (xQueueReceive(g_magnetometer_message_queue, &message, portMAX_DELAY) == pdPASS)
        {
            checking = true;

            float original_heading = get_heading();
            if (original_heading < 0)
                original_heading += 360;
            printf("Original heading: %2.2f\n", original_heading);
            float target_heading = original_heading + message;
            /* Put within 0 to 360 */
            if (target_heading < 0)
            {
                target_heading += 360;
            }
            else if (target_heading > 360)
            {
                target_heading -= 360;
            }

            printf("Target heading: %2.2f\n", target_heading);
            float min_heading_range = (float)target_heading - 1.5;

            if (min_heading_range < 0)
            {
                min_heading_range += 360;
            }
            else if (min_heading_range > 360)
            {
                min_heading_range -= 360;
            }

            float max_heading_range = (float)target_heading + 35.5;

            if (max_heading_range < 0)
            {
                max_heading_range += 360;
            }
            else if (max_heading_range > 360)
            {
                max_heading_range -= 360;
            }
            // Start checking
            float initial_heading = get_heading();
            if (initial_heading > target_heading)
            {
                target_heading += 360;
            }
            rotate_right_for_ticks(100, 999, 999);
            while (checking)
            {
                float heading = get_heading();

                if (heading < 0)
                    heading += 360;

                printf("Heading: %.2f\n\n", heading);
                vTaskDelay(pdMS_TO_TICKS(300));

                if (min_heading_range < max_heading_range ? heading > min_heading_range && heading < max_heading_range : heading > min_heading_range || heading < max_heading_range)
                {

                    stop();
                    xQueueSend(g_maze_message_queue, &heading, portMAX_DELAY);

                    checking = false;
                }
            }
        }
    }
}
void init_magnetometer()
{
    g_magnetometer_message_queue = xQueueCreate(3, sizeof(int));
    g_maze_message_queue = xQueueCreate(3, sizeof(int));
    printf("Configuring magnetometer\n");
    configure_magnetometer();
    printf(" Configure done ");
    xTaskCreate(magnetometer_task,
                "Magnetometer Task",
                configMINIMAL_STACK_SIZE,
                (void *)1,
                tskIDLE_PRIORITY,
                &g_magnetometer_task_handle);

    printf("Magnetometer initialized\n");
}
#endif