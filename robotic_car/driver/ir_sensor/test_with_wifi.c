
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task_message_buffer.h"
#include "barcode_driver.c"
#include "../wifi/main.c"

#undef BARCODE_UNIT_TESTING
#define DISABLE_WIFI_MAIN
int main()
{

    
    main_2();

    while(true);
}