#include "PointQueue.h"
#include "uart.h"
#include <stdio.h>

typedef struct _point {
  int16_t x;
  int16_t y;
}
point_t;
static point_t queue_data[POINT_QUEUE_SIZE];
static uint32_t queue_head;
static uint32_t queue_size;
static uint32_t queue_tail;
bool queue_dequeue(int16_t * x_out, int16_t * y_out) {
  if (queue_isempty()) return false;
  * x_out = queue_data[queue_tail].x;
  * y_out = queue_data[queue_tail].y;
  queue_tail = (queue_tail + 1) % POINT_QUEUE_SIZE;
  queue_size--;
  return true;
}
bool queue_enqueue(int16_t x, int16_t y) {
  if (queue_isfull()) return false;
  queue_data[queue_head].x = x;
  queue_data[queue_head].y = y;
  queue_head = (queue_head + 1) % POINT_QUEUE_SIZE;
  queue_size++;
  return true;
}
void queue_init(void) {
  queue_head = 0;
  queue_size = 0;
  queue_tail = 0;
}
bool queue_isempty(void) {
  return (queue_size == 0);
}
bool queue_isfull(void) {
  return (queue_size == POINT_QUEUE_SIZE);
}