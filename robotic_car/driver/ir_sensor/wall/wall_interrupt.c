#include "wall_interrupt.h"
#include "decider.c"

void wall_edge_irq(uint gpio, uint32_t events){
    static BaseType_t taskPriorityl;
    DeciderMessage_t message;
    if (events ==  GPIO_IRQ_EDGE_RISE ) message.data = 1;
    else message.data = 0;
    if (gpio == LEFT_IR_PIN){
        message.type = D_WALL_LEFT_EVENT;
        xQueueSendFromISR(g_decider_message_queue, &message, &taskPriorityl );
    }
    else if (gpio == RIGHT_IR_PIN){
        
        message.type = D_WALL_RIGHT_EVENT;
        xQueueSendFromISR(g_decider_message_queue, &message, &taskPriorityl );
    }
}

#ifdef WALL_UNIT_TEST_
int main(){
    

    stdio_init_all();

    //init_decider();

    /* Put this in main() */
    
    //gpio_set_irq_enabled(LEFT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    //gpio_set_irq_enabled(RIGHT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    /* Call barcode_edge_irq in a generic isr instead */
    gpio_set_irq_callback(&wall_edge_irq);

}
#endif