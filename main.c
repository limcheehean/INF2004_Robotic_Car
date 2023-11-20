#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "lwipopts.h"
#include "lwip/apps/httpd.h"
#include "ssi.h"
#include "cgi.h"

// WIFI Credentials - take care if pushing to github!
const char WIFI_SSID[] = "POCO F4 GT";
const char WIFI_PASSWORD[] = "qqsypbcppz7dt4m";
// 2201427@sit.singaporetech.edu.sg
// Kanagarani2!
// S20,reness10
//ravirani, kanagarani18
int main()
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

    // Infinite loop
    while (1);
}