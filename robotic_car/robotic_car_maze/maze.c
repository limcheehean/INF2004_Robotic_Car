#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>
#include <pico/printf.h>
#include "../robotic_car/driver/motor/motor_controller.h"

#define MAZE_IN_FREERTOS_TASK

#ifndef ROBOTIC_MAZE_DEFINE
#define ROBOTIC_MAZE_DEFINE
#define MAP_WIDTH 6
#define MAP_HEIGHT 4
#define START_X 5
#define START_Y 1
#define END_X 0
#define END_Y 2
#endif

#define MAP_WIDTH 6
#define MAP_HEIGHT 4
#define START_X 5
#define START_Y 1
#define END_X 0
#define END_Y 2

#define TICKS_TO_ROTATE_90 25
#define TICKS_TO_MOVE_FORWARD 25

typedef struct node {
    struct node *next, *prev;
    struct block *block;
} node;

typedef struct block {
    int x, y, distance_from;
    bool visited;
    struct block *visited_from;
    node *neighbour;
} block;

typedef struct map {
    // Current coordinates of the car
    int current_x, current_y;

    /**
     * 0 - left
     * 1 - up
     * 2 - right
     * 3 - down
     */
    int orientation;

    // Store the grid of blocks on the map
    struct block blocks[MAP_WIDTH][MAP_HEIGHT];

    // Store node for djikstra queue
    struct node * head;
} map;

TaskHandle_t g_maze_task_handle;

void print_list(node * head) {
    node *current = head;
    while (current != NULL) {
        printf("Item (%d, %d)\n", current->block->x, current->block->y);
        current = current->next;
    }
}

void add_node(node **head, block *block) {
    // Create node
    node *current = *head;
    node *temp = (node*)malloc(sizeof(node));
    temp->block = block;
    temp->next = NULL;
    temp->prev = NULL;

    // First node
    if (*head == NULL) {
        *head = temp;
        return;
    }

    // Move to last node
    while (current->next != NULL)
        current = current->next;

    // Link new node to last node
    current->next = temp;
    temp->prev = current;
}

void add_node_to_head(node ** head, block * block) {
    // Create node
    node *current = *head;
    node *temp = (node*)malloc(sizeof(node));
    temp->block = block;
    temp->next = NULL;
    temp->prev = NULL;

    if (*head != NULL) {
        temp->next = current;
        current->prev = temp;
    }

    *head = temp;
}

block * dequeue(node ** head) {
    node *temp = *head;
    *head = temp->next;
    return temp->block;
}

struct map * get_map() {
    static struct map map;
    return &map;
}

// To implement with car control functions
bool left_is_wall() {
    struct map * map = get_map();
    int x = map->current_x;
    int y = map->current_y;
    if (y == 0)
        return x == 0 || x == 1;
    if (y == 1)
        return x == 0 || x == 3 || x == 4;
    if (y == 2)
        return x == 2 || x == 3 || x == 4 || x == 0;
    if (y == 3)
        return !(x == 2 || x == 5);
    return false;
}

bool right_is_wall() {
    struct map * map = get_map();
    int x = map->current_x;
    int y = map->current_y;
    if (y == 0)
        return x == 0 || x == 5;
    if (y == 1)
        return x == 2 || x == 3 || x == 5;
    if (y == 2)
        return !(x == 0 || x == 4);
    if (y == 3)
        return !(x == 4 || x == 1);
    return false;
}

bool front_is_wall() {
    struct map * map = get_map();
    int x = map->current_x;
    int y = map->current_y;
    if (y == 0)
        return true;
    if (y == 1)
        return x == 2 || x == 4;
    if (y == 2)
        return x == 1 || x == 4;
    if (y == 3)
        return x == 1 || x == 5;
    return false;
}

bool back_is_wall() {
    struct map * map = get_map();
    int x = map->current_x;
    int y = map->current_y;
    if (y == 0)
        return x == 2 || x == 4;
    if (y == 1)
        return x == 1 || x == 4;
    if (y == 2)
        return x == 1 || x == 5;
    if (y == 3)
        return true;
    return false;
}

void move_to_block(struct block *block) {
    map * map = get_map();
    printf("Moving car to (%d, %d)\n", block->x, block->y);
    // start off facing left
    int turn_ticks = 0;
    /**
     * x decrease = left;
     * x increase = right;
     * y increase = down;
     * y decrease = up;
     */
    int change_x = block->x - map->current_x;
    int target_angle = 0;
    if (block -> x < map->current_x){
        target_angle = 0;
    }
    else if (block -> x > map->current_y){
        target_angle = 2;
    }
    else if (block -> y > map->current_y){
        target_angle = 3;
    }
    else if (block -> y < map->current_y){
        target_angle = 1;
    }

    // 0 - 3 = -3
    // 4 - 3 = 1
    turn_ticks = (target_angle - map->orientation);
    if (turn_ticks < 0){
        turn_ticks = 4 + turn_ticks;
    }
    turn_ticks *= TICKS_TO_ROTATE_90;
    map->orientation = target_angle;
    
    if (turn_ticks > 0){
        printf("TURNING RIGHT FOR %d TICKS!\n", turn_ticks);
        rotate_right_for_ticks(100,turn_ticks/2, turn_ticks/2);
    }
    #ifdef MAZE_IN_FREERTOS_TASK
    vTaskDelay(pdMS_TO_TICKS(3000));
    #endif

    printf("MOVING FOrWARD FOR %d TICKS!\n", TICKS_TO_MOVE_FORWARD);
    move_forward_for_ticks(100,100,TICKS_TO_MOVE_FORWARD, TICKS_TO_MOVE_FORWARD);
    
    //sleep_ms(4000);
    #ifdef MAZE_IN_FREERTOS_TASK
    vTaskDelay(pdMS_TO_TICKS(3000));
    #endif
    map->current_x = block->x;
    map->current_y = block->y;

    
}

void dfs(struct block *block) {
    
    map * map = get_map();
    int x = block->x;
    int y = block->y;

    map->current_x = x;
    map->current_y = y;
    
    // Check for neighbour
    if (!left_is_wall())
        add_node(&block->neighbour, &map->blocks[x - 1][y]);

    if (!right_is_wall())
        add_node(&block->neighbour, &map->blocks[x + 1][y]);

    if (!front_is_wall())
        add_node(&block->neighbour, &map->blocks[x][y - 1]);

    if (!back_is_wall())
        add_node(&block->neighbour, &map->blocks[x][y + 1]);

    block->visited = true;

    // Go to each neighbour
    node * neighbour = block->neighbour;
    while (neighbour != NULL) {
        if (!neighbour->block->visited) {
            // Move to neighbour
            move_to_block(neighbour->block);
            dfs(neighbour->block);
            // Return from neighbour
            move_to_block(block);
        }
        neighbour = neighbour->next;
    }

}

void start_mapping() {
    
    map * map = get_map();
    
    // Initialise all blocks
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            block * block = &map->blocks[x][y];
            block->x = x;
            block->y = y;
        }
    }
    
    // Get starting block
    block * block = &map->blocks[START_X][START_Y];
    
    // Perform dfs recursively
    dfs(block);

    printf("Mapping Complete!\n");
    
}

void start_navigation() {

    map * map = get_map();

    // Reset visited for all blocks
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            block * block = &map->blocks[x][y];
            block->visited = false;
            block->distance_from = 1000;
            block->visited_from = NULL;
        }
    }

    // Get starting block
    block * block = &map->blocks[START_X][START_Y];
    block->distance_from = 0;
    add_node(&map->head, block);


    while (map->head != NULL) {
        block = dequeue(&map->head);
        //printf("(%d, %d)\n", block->x, block->y);
        block->visited = true;
        node * current = block->neighbour;
        while (current != NULL) {
            struct block * neighbour = current->block;
            //printf("Neighbour(%d, %d)\n", neighbour->x, neighbour->y);
            int distance = block->distance_from + 1;
            if (neighbour->distance_from > distance) {
                neighbour->distance_from = distance;
                neighbour->visited_from = block;
            }
            if (!neighbour->visited)
                add_node(&map->head, neighbour);
            current = current->next;
        }
    }



    // Get target block
    block = &map->blocks[END_X][END_Y];
    add_node_to_head(&map->head, block);
    while (block->x != START_X || block->y != START_Y) {
        block = block->visited_from;
        add_node_to_head(&map->head, block);
    }

    node * current = map->head;
    while (current != NULL) {
        move_to_block(current->block);
        current = current->next;
    }

    printf("Navigation Complete!\n");

    char output_string[1000] = "";
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            block = &map->blocks[x][y];
            sprintf(output_string, "%s (%d,%d)", output_string, block->x, block->y);
            node * neighbour = block->neighbour;
            while (neighbour != NULL) {
                sprintf(output_string, "%s [%d,%d]", output_string, neighbour->block->x, neighbour->block->y);
                neighbour = neighbour->next;
            }
        }
    }
    printf("%s\n", output_string);

}


void maze_task( void *pvParameters ) {

    start_mapping();
    vTaskDelete(NULL);
}

void init_maze_task(){
    //g_decider_message_queue = xQueueCreate(30, sizeof(DeciderMessage_t));
    
    xTaskCreate(maze_task,
                "Maze Task",
                configMINIMAL_STACK_SIZE,
                ( void * ) 0, // Can try experimenting with parameter
                tskIDLE_PRIORITY,
                &g_maze_task_handle);
    
    printf("Maze Task initialized\n");
}