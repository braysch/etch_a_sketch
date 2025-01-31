#include "Motor.h"
#define STEP_DELAY 3000
#define X_A (1 << 7)
#define X_B (1 << 6)
#define X_AN (1 << 2)
#define X_BN (1 << 3)
#define Y_A (1 << 11)
#define Y_B (1 << 10)
#define Y_AN (1 << 12)
#define Y_BN (1 << 13)
// Clockwise - A, B, A#, B#
static uint32_t xTable[8] = { X_A, X_A | X_B, X_B, X_B | X_AN, X_AN, X_AN | X_BN, X_BN, X_BN | X_A
,→ };
static uint32_t yTable[8] = { Y_A, Y_A | Y_B, Y_B, Y_B | Y_AN, Y_AN, Y_AN | Y_BN, Y_BN, Y_BN | Y_A
,→ };
static void x_stepClockwise()
{
for(uint8_t i = 0; i <= 7; i++)
{
GPIOB->ODR &= 0xFFFFFF33;
GPIOB->ODR |= xTable[i];
for(volatile int k = 0; k < STEP_DELAY; k++){}
}
GPIOB->ODR &= 0xFFFFFF33;
}
static void x_stepCounterClockwise()
{
for(uint8_t i = 0; i <= 7; i++)
{
GPIOB->ODR &= 0xFFFFFF33;
GPIOB->ODR |= xTable[7 - i];
for(volatile int k = 0; k < STEP_DELAY; k++){}
}
GPIOB->ODR &= 0xFFFFFF33;
}
static void y_stepClockwise()
{
for(uint8_t i = 0; i <= 7; i++)
{
GPIOE->ODR &= 0xFFFFC3FF;
GPIOE->ODR |= yTable[i];
for(volatile int k = 0; k < STEP_DELAY; k++){}
}
GPIOE->ODR &= 0xFFFFC3FF;
}
static void y_stepCounterClockwise()
{
for(uint8_t i = 0; i <= 7; i++)
{
GPIOE->ODR &= 0xFFFFC3FF;
GPIOE->ODR |= yTable[7-i];
for(volatile int k = 0; k < STEP_DELAY; k++){}
}
GPIOE->ODR &= 0xFFFFC3FF;
}
void x_Motor_Drive(bool direction)
{
if (!direction) // Clockwise
{
x_stepClockwise();
}
else // Counter-clockwise
{
x_stepCounterClockwise();
}
}
void y_Motor_Drive(bool direction)
{
if (!direction) // Clockwise
{
y_stepClockwise();
}
else // Counter-clockwise
{
y_stepCounterClockwise();
}
}