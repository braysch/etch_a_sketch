#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024

void uart_init(void);
uint32_t uart_getline(char *buffer, uint32_t buffer_len);
uint32_t uart_transmit(char *data, uint32_t len);
uint32_t uart_transmit_str(char *data);

#endif // UART_H
