#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>
#include <pico/printf.h>
#include <stdlib.h>
#include <hardware/gpio.h>

#define LEFT 0
#define FRONT 1
#define RIGHT 2
#define BACK 3
#define RESPONSE_NULL 0
#define RESPONSE_YES 1
#define RESPONSE_NO 2

// Node to be built into linked list
typedef struct node {
    struct node *next, *prev;
    struct block *block;
} node;

// Struct to store each block in map
typedef struct block {
    int x, y, distance_from;
    bool visited, shortest_path;
    struct block *visited_from;
    node *neighbour;
} block;

// Struct to store maze map
typedef struct map {
    // Current coordinates of the car
    int current_x, current_y;

    // Store the grid of blocks on the map
    struct block blocks[MAP_WIDTH][MAP_HEIGHT];

    // Store node for djikstra queue
    struct node * head;

    // Store car orientation
    int car_direction;
} map;

// Struct to store response for wall check questions
typedef struct response {
    int response;
} response;

// Add node to end of linked list
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

// Add node to start of linked list
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

// Pop off the first node in linked list
block * dequeue(node ** head) {
    node *temp = *head;
    *head = temp->next;
    return temp->block;
}

struct map * get_map() {
    static struct map map;
    return &map;
}

struct response * get_response() {
    static struct response response;
    return &response;
}

// Wait until a button is pressed on the Maker Pi Pico
bool wait_for_response() {
    response * response = get_response();
    while (response->response == RESPONSE_NULL) {}
    bool answer = response->response == RESPONSE_YES;
    response->response = RESPONSE_NULL;
    printf(answer == true ? "YES\n": "NO\n");
    return answer;
}

// To implement with car control functions
bool left_is_wall() {
    struct map * map = get_map();

    if ((map->current_x == START_X && map->current_y == START_Y && ENTRY_DIRECTION == LEFT) || (map->current_x == END_X && map->current_y == END_Y && EXIT_DIRECTION == LEFT))
        return true;

    if (DEMO_MODE == 1) {
        switch (map->car_direction) {
            case LEFT:
                printf("Is a wall in front of you?\n");
                break;
            case RIGHT:
                return false;
            case FRONT:
                printf("Is a wall on your left?\n");
                break;
            case BACK:
                printf("Is a wall on your right?\n");
                break;
        }
        return wait_for_response();
    }

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

    if ((map->current_x == START_X && map->current_y == START_Y && ENTRY_DIRECTION == RIGHT) || (map->current_x == END_X && map->current_y == END_Y && EXIT_DIRECTION == RIGHT))
        return true;

    if (DEMO_MODE == 1) {
        switch (map->car_direction) {
            case LEFT:
                return false;
            case RIGHT:
                printf("Is a wall in front of you?\n");
                break;
            case FRONT:
                printf("Is a wall on your right?\n");
                break;
            case BACK:
                printf("Is a wall on your left?\n");
                break;
        }
        return wait_for_response();
    }

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

    if ((map->current_x == START_X && map->current_y == START_Y && ENTRY_DIRECTION == FRONT) || (map->current_x == END_X && map->current_y == END_Y && EXIT_DIRECTION == FRONT))
        return true;

    if (DEMO_MODE == 1) {
        switch (map->car_direction) {
            case LEFT:
                printf("Is a wall on your right?\n");
                break;
            case RIGHT:
                printf("Is a wall on your left?\n");
                break;
            case FRONT:
                printf("Is a wall in front of you?\n");
                break;
            case BACK:
                return false;
        }
        return wait_for_response();
    }

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

    if ((map->current_x == START_X && map->current_y == START_Y && ENTRY_DIRECTION == BACK) || (map->current_x == END_X && map->current_y == END_Y && EXIT_DIRECTION == BACK))
        return true;

    if (DEMO_MODE == 1) {
        switch (map->car_direction) {
            case LEFT:
                printf("Is a wall to your left?\n");
                break;
            case RIGHT:
                printf("Is a wall on your right?\n");
                break;
            case FRONT:
                return false;
            case BACK:
                printf("Is a wall in front of you?\n");
                break;
        }
        return wait_for_response();
    }

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

    if (DEMO_MODE == 0)
        printf("Moving car to (%d, %d)\n", block->x, block->y);

    int target_direction = 0;
    if (block->x == map->current_x - 1)
        target_direction = LEFT;
    else if (block->x == map->current_x + 1)
        target_direction = RIGHT;
    else if (block->y == map->current_y - 1)
        target_direction = FRONT;
    else if (block->y == map->current_y + 1)
        target_direction = BACK;

    int rotation = target_direction - map->car_direction;

    if (DEMO_MODE == 1) {
        if (rotation == -1 || rotation == 3)
            printf("Turn left then move forward\n");
        else if (rotation == 1 || rotation == -3)
            printf("Turn right then move forward\n");
        else if (abs(rotation) == 2)
            printf("Turn around then move forward\n");
        else
            printf("Move forward\n");
    }

    map->current_x = block->x;
    map->current_y = block->y;
    map->car_direction = target_direction;
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

// ISR to respond YES and NO for wall check questions
void press_button_isr(uint gpio, uint32_t events) {
    response * response = get_response();
    response->response = gpio == 20? RESPONSE_YES: RESPONSE_NO;
}

void start_mapping() {

    // Initialise GPIO 20 & 21 as YES and NO buttons on the Maker Pi Pico
    gpio_set_irq_enabled_with_callback(20, GPIO_IRQ_EDGE_FALL,true,&press_button_isr);
    gpio_set_irq_enabled_with_callback(21, GPIO_IRQ_EDGE_FALL,true,&press_button_isr);

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
    map->car_direction = LEFT;
    
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

    // Perform Dijkstra
    while (map->head != NULL) {
        block = dequeue(&map->head);
        block->visited = true;
        node * current = block->neighbour;
        while (current != NULL) {
            struct block * neighbour = current->block;
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



    // Build shortest path
    block = &map->blocks[END_X][END_Y];
    block->shortest_path = true;
    add_node_to_head(&map->head, block);
    while (block->x != START_X || block->y != START_Y) {
        block = block->visited_from;
        block->shortest_path = true;
        add_node_to_head(&map->head, block);
    }

    // Build shortest path string
    char shortest_path[1000] = "";
    node * current = map->head;
    while (current != NULL) {
        sprintf(shortest_path, "%s(%d, %d)", shortest_path, current->block->x, current->block->y);
        move_to_block(current->block);
        if (current->next != NULL)
            sprintf(shortest_path, "%s -> ", shortest_path);
        current = current->next;
    }

    printf("Shortest Path: %s\n", shortest_path);

    printf("Navigation Complete!\n");

    // Generate json to copy into mapping display
    char output_string[2000] = "[";
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            block = &map->blocks[x][y];
            sprintf(output_string, "%s{\"x\": %d, \"y\":%d, \"shortest_path\":%d, \"neighbours\": [", output_string, block->x, block->y, block->shortest_path);
            node * neighbour = block->neighbour;
            while (neighbour != NULL) {
                sprintf(output_string, "%s{\"x\": %d, \"y\":%d}", output_string, neighbour->block->x, neighbour->block->y);
                if (neighbour->next != NULL)
                    sprintf(output_string, "%s, ", output_string);
                neighbour = neighbour->next;
            }
            sprintf(output_string, "%s]}", output_string);
            if (x != MAP_WIDTH - 1 || y != MAP_HEIGHT - 1)
                sprintf(output_string, "%s, ", output_string);
        }
    }
    sprintf(output_string, "%s]", output_string);
    printf("%s\n", output_string);

}