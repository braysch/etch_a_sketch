#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l476xx.h"

#define STEP_DELAY 3000

// Function prototypes
void x_Motor_Drive(bool direction);
void y_Motor_Drive(bool direction);

#endif // MOTOR_H
