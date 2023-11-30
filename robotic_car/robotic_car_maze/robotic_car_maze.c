
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

// Main function
int main()
{

    // Initialize USB for standard I/O
    stdio_usb_init();

    // Wait for 9000 milliseconds (9 seconds) to allow USB initialization
    sleep_ms(9000);

    // Start the mapping process
    start_mapping();

    // Start the navigation process
    start_navigation();

    // Infinite loop to keep the program running
    while (true);
}