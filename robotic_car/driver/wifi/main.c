#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "lwipopts.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"
#include "cgi.h"
#include "wifi_task_message_buffer.h"

#ifndef PICO_MAX_SHARED_IRQ_HANDLERS
#define PICO_MAX_SHARED_IRQ_HANDLERS 4u
#endif

// WIFI Credentials - replace it with current user's credentials
const char WIFI_SSID[] = "WIFISSID";
const char WIFI_PASSWORD[] = "WIFIPASSWORD";

// Entry point for the application
int main_2()
{
    // Initialize USB serial communication
    stdio_usb_init();

    // Initialize CYW43 and enable sta mode
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

    // Create a message queue for communication with the WiFi task
    g_wifi_task_message_queue = xQueueCreate(5, sizeof(WifiTaskMessage_t));

    if (g_wifi_task_message_queue == NULL)
    {
        printf("Error creating g_wifi_task_message_queue\n");
    }

    // Create a task for receiving WiFi task messages
    if (xTaskCreate(wifi_task_message_receive_task, "Wifi CurrentMessage Task", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY + 1, &g_wifi_task_message_task_handle) != pdPASS)
    {
        printf("Error creating wifi_task_message_receive_task\n");
    }

    // Create a message queue for concatenated messages
    g_concatenatedMessagesQueue = xQueueCreate(1, sizeof(WifiTaskMessage_t));

    if (g_concatenatedMessagesQueue == NULL)
    {
        printf("Error creating g_concatenatedMessagesQueue\n");
    }

// Conditionally create a task for receiving WiFi task test data messages
#ifndef DISABLE_WIFI_MAIN
    if (xTaskCreate(wifi_task_message_receive_task_testData, "Wifi totalMessage Task", configMINIMAL_STACK_SIZE, (void *)0, tskIDLE_PRIORITY, &g_wifi_task_message_task_handle_test) != pdPASS)
    {
        printf("Error creating wifi_task_message_receive_task_test\n");
    }
#endif
}

#ifndef DISABLE_WIFI_MAIN
int main()
{
    main_2();
    vTaskStartScheduler();

    while (1);
}
#endif
