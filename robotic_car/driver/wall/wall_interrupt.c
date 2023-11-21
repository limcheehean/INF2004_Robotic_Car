#include "wall_interrupt.h"


void barcode_edge_irq(uint gpio, uint32_t events){

    if (gpio == LEFT_IR_PIN){

    }
    else if (gpio == RIGHT_IR_PIN){
        
    }
}

int main(){

    stdio_init_all();

    /* Put this in main() */
    gpio_set_irq_enabled(LEFT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(RIGHT_IR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    /* Call barcode_edge_irq in a generic isr instead */
    gpio_set_irq_callback(&barcode_edge_irq);

}