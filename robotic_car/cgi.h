#include "lwip/apps/httpd.h"
#include "driver/motor/motor_controller.h"
#include "pico/cyw43_arch.h"

const char *cgi_move_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

    printf("Moved instruction received\n");

    if (strcmp(pcParam[0], "move") == 0 &&
        strcmp(pcParam[1], "left_power") == 0 &&
        strcmp(pcParam[2], "right_power") == 0)
    {

        float left_power = atoi(pcValue[1]) / 100.0f;
        float right_power = atoi(pcValue[2]) / 100.0f;

        if (strcmp(pcValue[0], "up") == 0)
        {
            printf("[FORWARD] Left: %.2f, Right: %.2f\n", left_power, right_power);
            move_forward(left_power, right_power);
        }
        else if (strcmp(pcValue[0], "down") == 0)
        {
            printf("[BACKWARD] Left: %.2f, Right: %.2f\n", left_power, right_power);
            move_backward(left_power, right_power);
        }
        else if (strcmp(pcValue[0], "left") == 0)
        {
            printf("[LEFT]\n");
            turn_left(1);
        }
        else if (strcmp(pcValue[0], "right") == 0)
        {
            printf("[RIGHT]\n");
            turn_right(1);
        }
        else if (strcmp(pcValue[0], "stop") == 0)
        {
            printf("[STOP]\n");
            stop();
        }
    }
    return "successful";
}

// Wifi
//  CGI handler which is run when a request for /led.cgi is detected
const char *cgi_led_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if an request for LED has been made (/led.cgi?led=x)
    if (strcmp(pcParam[0], "led") == 0)
    {
        // Look at the argument to check if LED is to be turned on (x=1) or off (x=0)
        if (strcmp(pcValue[0], "0") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        else if (strcmp(pcValue[0], "1") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }

    // Send the index page back to the user
    return "/index.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlersWifi[] = {
    {
        // Html request for "/led.cgi" triggers cgi_handler
        "/led.cgi", cgi_led_handler
    },
};

static const tCGI cgi_handlers[] = {
    {"/move.cgi", cgi_move_handler},
    };

void cgi_init()
{
    http_set_cgi_handlers(cgi_handlers, 1);
    http_set_cgi_handlers(cgi_handlersWifi, 1);
}
