#include "stm32l476xx.h"
#include "Motor.h"
#include "Draw.h"
#include "commands.h"
#include "parser.h"
#include "PointQueue.h"
#include "uart.h"
#include "main.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char DebugBuffer[DEBUG_BUFFER_SIZE];
static void PinInitializaiton() {
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //Enable A clock
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOEEN; //Enable E clock
  RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN; //Enable B clock
  //Configure MODER
  GPIOA -> MODER &= 0xFFFFF303; // set pins 1, 2, 3, and 5 of port A to 00, input.
  GPIOE -> MODER &= 0xC00FFFFF;
  GPIOE -> MODER |= 0x15500000; // set pins 10, 11, 12, and 13 of port E to 01, output.
  GPIOB -> MODER &= 0xFFFF0F0F;
  GPIOB -> MODER |= 0x00005050; //set pins 2, 3, 6, 7, of Port B to output.
}
// Buffer to hold the current command
#define COMMAND_BUFFER_SIZE 1024
static char command_buffer[COMMAND_BUFFER_SIZE];
static void handle_command(char * command);
static int16_t x_max = 2250; // set bounds
static int16_t y_max = 1500;
static int16_t x_prev = 0; // set beginning reference point
static int16_t y_prev = 0;
int main(void) {
  // Switch system clock to HSI here
  RCC -> CR |= RCC_CR_HSION;
  while ((RCC -> CR & RCC_CR_HSIRDY) == 0);
  RCC -> CFGR = (RCC -> CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
  PinInitializaiton();
  queue_init();
  uart_init();
  // Setup initial state
  memset(command_buffer, 0, sizeof(command_buffer));
  while (true) {
    uint32_t command_len = uart_getline(command_buffer, COMMAND_BUFFER_SIZE);
    if (command_len != 0) // We got a command
    {
      handle_command(command_buffer);
    }
    __WFI(); // Wait for something to happen
  }
  // main loop
}
static void handle_command(char * command) {
  // Parse the command type
  command_type_t command_type = parser_get_command_type(command);
  switch (command_type) {
    // Syntax: P [X] [Y]
    // X: 16-bit unsigned integer coordinate
    // Y: 16-bit unsigned integer coordinate
    //
    // Adds a single point to the point queue.
  case COMMAND_POINT: {
    char x_field[10];
    char y_field[10];
    if (parser_get_field(command, 1, x_field, sizeof(x_field)) == 0) {
      uart_transmit_str(">P Bad X\r\n");
      return;
    }
    if (parser_get_field(command, 2, y_field, sizeof(y_field)) == 0) {
      uart_transmit_str(">P Bad Y\r\n");
      return;
    }
    int16_t x_coord = (int16_t) strtol(x_field, NULL, 10);
    int16_t y_coord = (int16_t) strtol(y_field, NULL, 10);
    if (!queue_enqueue(x_coord, y_coord)) {
      uart_transmit_str(">P Queue full\r\n");
      return;
    }
    uart_transmit_str(">P\r\n");
    break;
  }
  case COMMAND_HOME:
    uart_transmit_str(">H\r\n");
    break;
    // Syntax: D
    //
    // Traces through the position queue.
  case COMMAND_DRIVE:
    uart_transmit_str(">D\r\n");
    int16_t x;
    int16_t y;
    while (queue_dequeue( & x, & y)) {
      // Drive to (x, y)
      if (x > x_max) {
        x = x_max;
      }
      if (y > y_max) {
        y = y_max;
      }
      SEND_DEBUG("At %d, %d. Going to %d, %d.\r\n", x_prev, y_prev, x, y);
      Draw(x, y, x_prev, y_prev);
      uart_transmit_str("Draw done\r\n");
      x_prev = x; // update cursor
      y_prev = y;
    }
    // Send queue empty message
    uart_transmit_str("E\r\n");
    break;
  case COMMAND_INVALID:
    uart_transmit_str(">E Bad command\r\n");
    break;
  }
}