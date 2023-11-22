#ifndef DECIDER_HEADER_
#define DECIDER_HEADER

#define D_WALL_LEFT_EVENT 0 
#define D_WALL_RIGHT_EVENT 1
#define D_BARCODE_EVENT 2
#define D_ULTRASONIC_EVENT 3
#define D_WALL_TESTING 4
#define D_BARCODE_TESTING 5
#define D_TURNING 6

typedef struct {
    int type;
    int data;
} DeciderMessage_t;



#endif