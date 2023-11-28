#ifndef CGI_HEADER_
#define CGI_HEADER_
#include "lwip/apps/httpd.h"

#include "driver/motor/motor_controller.h"
#include "pico/cyw43_arch.h"

// CGI handler which is run when a request for /led.cgi is detected
const char * cgi_led_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if an request for LED has been made (/led.cgi?led=x)
    if (strcmp(pcParam[0] , "led") == 0){
        // Look at the argument to check if LED is to be turned on (x=1) or off (x=0)
        if(strcmp(pcValue[0], "0") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        else if(strcmp(pcValue[0], "1") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
    
    // Send the index page back to the user
    return "/index.shtml";
}

const char *cgi_move_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

    printf("Moved instruction received\n");

    if (strcmp(pcParam[0], "move") == 0 &&
        strcmp(pcParam[1], "left_power") == 0 &&
        strcmp(pcParam[2], "right_power") == 0)
    {

        float left_power = atoff(pcValue[1]);
        float right_power = atoff(pcValue[2]);
        int left_ticks = atoi(pcValue[3]);
        int right_ticks = atoi(pcValue[4]);

        if (strcmp(pcValue[0], "up") == 0)
        {
            stop();
            printf("[FORWARD] Left: %.2f (%d ticks), Right: %.2f (%d ticks)\n", left_power, left_ticks, right_power, right_ticks);
            // move_forward(left_power, right_power);
            move_forward_for_ticks(left_power, right_power, left_ticks, right_ticks);
        }
        else if (strcmp(pcValue[0], "down") == 0)
        {
            stop();
            printf("[BACKWARD] Left: %.2f (%d ticks), Right: %.2f (%d ticks)\n", left_power, left_ticks, right_power, right_ticks);
            move_backward_for_ticks(left_power, right_power, left_ticks, right_ticks);
        }
        else if (strcmp(pcValue[0], "left") == 0)
        {
            stop();
            printf("[LEFT] %.2f (%d ticks)\n", right_power, right_ticks);
            rotate_left_for_ticks(right_power, left_ticks, right_ticks);
        }
        else if (strcmp(pcValue[0], "right") == 0)
        {
            stop();
            printf("[RIGHT] %.2f (%d ticks)\n", left_power, left_ticks);
            rotate_right_for_ticks(left_power, left_ticks, right_ticks);
        }
        else if (strcmp(pcValue[0], "stop") == 0)
        {
            printf("[STOP]\n");
            stop();
        }
    }
    return "successful";
}

const char *cgi_demo_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {

    printf("Demo instruction received\n");

    if (strcmp(pcParam[0], "function") == 0)
    {
        if (strcmp(pcValue[0], "straight_path") == 0)
        {
            printf("Demonstrating straight path\n");
        }
        else if (strcmp(pcValue[0], "turn_left_right") == 0)
        {
            printf("Demonstrating turn left right\n");
        }
        else if (strcmp(pcValue[0], "barcode") == 0)
        {
            printf("Demonstrating barcode\n");
        }
        else if (strcmp(pcValue[0], "obstacle") == 0)
        {
            printf("Demonstrating obstacle\n");

        }
    }
    return "successful";
}


// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    // {
    //     // Html request for "/led.cgi" triggers cgi_handler
    //     "/led.cgi", cgi_led_handler
    // },
    {"/move.cgi", cgi_move_handler},
    {"/demo.cgi", cgi_demo_handler}
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 2);
}
#endif