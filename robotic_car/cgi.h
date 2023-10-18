#include "lwip/apps/httpd.h"
#include "motor_controller.h"

const char * cgi_move_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {

    printf("Moved instruction received\n");

    if (strcmp(pcParam[0], "move") == 0) {

        if (strcmp(pcValue[0], "up") == 0) {
            printf("[FORWARD]\n");
            motor_set_direction(MOTOR_DIR_FRONT);
            motor_move(10000);
        } else if (strcmp(pcValue[0], "down") == 0) {
            printf("[BACKWARD]\n");
            motor_set_direction(MOTOR_DIR_BACK);
            motor_move(10000);
        } else if (strcmp(pcValue[0], "left") == 0) {
            printf("[LEFT]\n");
            motor_set_direction(MOTOR_DIR_LEFT);
            motor_move(20);
        } else if (strcmp(pcValue[0], "right") == 0) {
            printf("[RIGHT]\n");
            motor_set_direction(MOTOR_DIR_RIGHT);
            motor_move(20);
        } else if (strcmp(pcValue[0], "stop") == 0) {
            printf("[STOP]\n");
            motor_stop();
        }


    }
    return "successful";
}

static const tCGI cgi_handlers[] = {
        {"/move.cgi", cgi_move_handler}
};


void cgi_init() {
    http_set_cgi_handlers(cgi_handlers, 1);
}