#ifndef SSI_HEADER_
#define SSI_HEADER_
#include "hardware/adc.h"
#include "wifi_task_message_buffer.h"

// SSI tags - tag length limited to 8 bytes by default
const char *ssi_tags[] = {"volt", "temp", "type", "message"};

// SSI handler function
u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{

    size_t printed;
    switch (iIndex)
    {
    case 0: // volt
    {
        // Read voltage from ADC and convert to actual voltage
        const float voltage = adc_read() * 3.3f / (1 << 12);
        printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
    }
    break;
    case 1: // temp
    {
        // Read voltage from ADC and convert to temperature in Celsius
        const float voltage = adc_read() * 3.3f / (1 << 12);
        const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
        printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
    }
    break;
    case 2: // type
    {
        // Insert the message type
        printed = snprintf(pcInsert, iInsertLen, "%d", totalMessage.type);
    }
    break;
    case 3: // message
    {
        // Receive a message from the concatenated messages queue
        if (xQueueReceive(g_concatenatedMessagesQueue, &totalMessage, 0) == pdTRUE)
        {

            printed = snprintf(pcInsert, iInsertLen, "%s", totalMessage.message);
        }
        else
        {
            printed = snprintf(pcInsert, iInsertLen, "Queue reception failed");
        }
    }
    break;
    default:
        printed = 0;
        break;
    }

    return (u16_t)printed;
    //}
}

// Initialise the SSI handler
void ssi_init()
{
    // Initialise ADC (internal pin)
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    // Set the SSI handler function and tags
    http_set_ssi_handler(ssi_handler, ssi_tags, 4);
}

#endif
