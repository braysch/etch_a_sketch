#include "Motor.h"
#include "Draw.h"

#include <stdlib.h>

static bool prev_x_dir = true;
static bool prev_y_dir = true;
void Draw(int x, int y, int x_cursor, int y_cursor) {
  x = x - x_cursor; // get relative coordinates
  y = y - y_cursor;
  int curr_x = 0; // relative position
  int curr_y = 0;
  bool x_dir = prev_x_dir; // motor direction
  bool y_dir = prev_y_dir;
  if (x < 0) {
    x_dir = true;
  } else if (x > 0) {
    x_dir = false;
  } // set motor direction
  if (y < 0) {
    y_dir = true;
  } else if (y > 0) {
    y_dir = false;
  }
  if (x_dir != prev_x_dir) {
    for (int i = 0; i < 40; i++) {
      x_Motor_Drive(x_dir);
    }
  }
  if (y_dir != prev_y_dir) {
    for (int i = 0; i < 30; i++) {
      y_Motor_Drive(y_dir);
    }
  }
  prev_x_dir = x_dir;
  prev_y_dir = y_dir;
  int count_x = 0; // motor timers
  int count_y = 0;
  while (curr_x < abs(x) || curr_y < abs(y)) // wait until point has been reached
  {
    count_x++;
    count_y++;
    if (count_x >= abs(y) && x != 0) {
      x_Motor_Drive(x_dir);
      count_x = 0;
      curr_x++;
    }
    if (count_y >= abs(x) && y != 0) {
      y_Motor_Drive(y_dir);
      count_y = 0;
      curr_y++;
    }
  }
  x_cursor += x;
  x_cursor += y;
}