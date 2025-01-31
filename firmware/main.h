#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

#define DEBUG_BUFFER_SIZE 1024

// Function prototypes
void PinInitializaiton(void);
static void handle_command(char *command);

// Global variables
extern int16_t x_max;
extern int16_t y_max;
extern int16_t x_prev;
extern int16_t y_prev;

#endif // MAIN_H
