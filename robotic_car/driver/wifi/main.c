#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "lwipopts.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"
#include "cgi.h"
#ifndef WIFI_TASK_MESSAGE_HEADER
    #include "wifi_task_message_buffer.h"
    #define WIFI_TASK_MESSAGE_HEADER 1
#endif

// WIFI Credentials - take care if pushing to github!
const char WIFI_SSID[] = "ravirani"; //20V3
const char WIFI_PASSWORD[] = "kanagarani18";
// Kanagarani2!
// S20,reness10
//ravirani, kanagarani18

typedef struct {
    int someValue;
    char someString[20];
    // Add other members as needed
} TaskParameters;

int main_2()
{

    
    stdio_usb_init();

    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
    {
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");

    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");
    
    //wifi_task_message_receive_task(&taskParams);

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
//    cgi_init();
//    printf("CGI Handler initialised\n");
    
    //Initiating wifi_task_message_receive_task to initialize currentMessage
    //WifiTaskMessage_t taskParams = {42, "Hello, Task!"};
    // wifi_task_message_receive_task((void *)&taskParams);
    // printf("wifi initialised\n");
    // Infinite loop
}

#ifndef DISABLE_WIFI_MAIN 
int main(){
    main_2();

    while (1);
}
#endif