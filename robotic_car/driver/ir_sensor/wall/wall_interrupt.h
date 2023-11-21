#include "pico/stdlib.h"
#include <stdio.h>

#ifndef WALL_INTERRUPT_
#define WALL_INTERRUPT_
// Pin 15 - Left
// Pin 16 - Right

#define LEFT_IR_PIN 15
#define RIGHT_IR_PIN 16

/* Add this interrupt in the main function */
void wall_edge_irq(uint gpio, uint32_t events);

#endif
