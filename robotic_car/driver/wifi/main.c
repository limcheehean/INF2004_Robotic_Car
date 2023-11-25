#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "lwipopts.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"
#include "cgi.h"
//#include "wifi_task_message_receiver.c"
#include "wifi_task_message_buffer.h"
#define MAX_MESSAGES 100

#ifndef PICO_MAX_SHARED_IRQ_HANDLERS
#define PICO_MAX_SHARED_IRQ_HANDLERS 4u
#endif

// WIFI Credentials - take care if pushing to github!
//const char WIFI_SSID[] = "POCO F4 GT"; //20V3
//const char WIFI_PASSWORD[] = "qqsypbcppz7dt4m";
//const char WIFI_SSID[] = "Thrith";
//const char WIFI_PASSWORD[] = "reness10";
const char WIFI_SSID[] = "AndroidAP_2237";
const char WIFI_PASSWORD[] = "7322password";
// Kanagarani2!
// S20,reness10
// ravirani, kanagarani18

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

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");

    //    cgi_init();
    //    printf("CGI Handler initialised\n");

    g_wifi_task_message_queue = xQueueCreate(5, sizeof(WifiTaskMessage_t));

    if (g_wifi_task_message_queue == NULL)
    {
        printf("Error creating g_wifi_task_message_queue\n");
    }

    if (xTaskCreate(wifi_task_message_receive_task, "Wifi CurrentMessage Task", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY + 1, &g_wifi_task_message_task_handle) != pdPASS)
    {
        printf("Error creating wifi_task_message_receive_task\n");
    }

    g_concatenatedMessagesQueue = xQueueCreate(1, sizeof(MAX_MESSAGES));

    if (g_concatenatedMessagesQueue == NULL)
    {
        printf("Error creating g_concatenatedMessagesQueue\n");
    }

    #ifndef DISABLE_WIFI_MAIN
    if (xTaskCreate(wifi_task_message_receive_task_testData, "Wifi totalMessage Task", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY, &g_wifi_task_message_task_handle_test) != pdPASS)
    {
        printf("Error creating wifi_task_message_receive_task_test\n");
    }
    #endif
}

#ifndef DISABLE_WIFI_MAIN
int main(){
    main_2();
    vTaskStartScheduler();

    while (1)
        ;
}
#endif

// #ifndef DISABLE_WIFI_MAIN
// int main(){
//     main_2();

//     while (1);
// }
// #endif