#ifndef POINTQUEUE_H
#define POINTQUEUE_H

#include <stdint.h>
#include <stdbool.h>

#define POINT_QUEUE_SIZE 100 // Define queue size

// Function prototypes
void queue_init(void);
bool queue_enqueue(int16_t x, int16_t y);
bool queue_dequeue(int16_t *x_out, int16_t *y_out);
bool queue_isempty(void);
bool queue_isfull(void);

#endif // POINTQUEUE_H