#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "cgi.h"
#include "driver/motor/motor_controller.h"
#include "driver/encoder/wheel_encoder.h"

#define WIFI_SSID "POCO F4 GT"
#define WIFI_PASSWORD "qqsypbcppz7dt4m"

#define LEFT_MOTOR_PWM 0
#define RIGHT_MOTOR_PWM 1
#define LEFT_MOTOR_FORWARD 2
#define LEFT_MOTOR_BACKWARD 3
#define RIGHT_MOTOR_FORWARD 4
#define RIGHT_MOTOR_BACKWARD 5
#define LEFT_ENCODER_INPUT 6
#define RIGHT_ENCODER_INPUT 7
#define LEFT_ENCODER_POWER 28
#define RIGHT_ENCODER_POWER 22

void configure_wifi() {

    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    printf("Attempting to connect to %s\n", WIFI_SSID);
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
        printf("Connection failed, trying again...\n");
    }

    printf("Connected to %s successfully\n", WIFI_SSID);
}

// Temporary power for sensors, use 3V3 when splitter available
void configure_sensor_power(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_SIO);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, 1);
}

int main() {

    stdio_init_all();

    configure_wifi();
    //sleep_ms(5000);

    // Configure sensors using GPIO power
    configure_sensor_power(LEFT_ENCODER_POWER);
    configure_sensor_power(RIGHT_ENCODER_POWER);

    // Configure motor controller
    init_motor_controller(LEFT_MOTOR_PWM,
                          RIGHT_MOTOR_PWM,
                          LEFT_MOTOR_FORWARD,
                          RIGHT_MOTOR_FORWARD,
                          LEFT_MOTOR_BACKWARD,
                          RIGHT_MOTOR_BACKWARD);

    // Configure wheel encoder
    init_wheel_encoder(LEFT_ENCODER_INPUT,
                       RIGHT_ENCODER_INPUT);


    // Test move
//    move_forward(1);
//    sleep_ms(20000);
//    move_backward(1);
//    sleep_ms(20000);
//    turn_left(1);
//    sleep_ms(20000);
//    turn_right(1);
//    sleep_ms(20000);
//    stop();

    // Temporary web server for testing
    httpd_init();
    cgi_init();

    while (true);

}
