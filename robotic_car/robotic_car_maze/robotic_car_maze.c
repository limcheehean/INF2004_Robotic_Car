
#ifndef ROBOTIC_MAZE_DEFINE
#define ROBOTIC_MAZE_DEFINE
#define MAP_WIDTH 6
#define MAP_HEIGHT 4
#define START_X 5
#define START_Y 1
#define END_X 0
#define END_Y 2
#endif

#include <pico/time.h>
#include <pico/stdio_usb.h>
#include "maze.c"



int main() {
    stdio_usb_init();
    sleep_ms(9000);
    start_mapping();
    start_navigation();
    while (true);
}