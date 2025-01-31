#include "uart.h"
#include <string.h>
#define UART_RX_BUFFER_SIZE 1024
#define UART_RX_BUFFER_MASK 0x3FF
#define UART_TX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_MASK 0x3FF
// Receive buffer
static char rx_buffer[UART_RX_BUFFER_SIZE];
static uint32_t rx_size;
static uint32_t rx_head;
static uint32_t rx_tail;
#define PEEK_RX(index) rx_buffer[(rx_tail + (index)) & UART_RX_BUFFER_MASK]
// Transmit buffer
static char tx_buffer[UART_TX_BUFFER_SIZE];
static uint32_t tx_size;
static uint32_t tx_head;
static uint32_t tx_tail;
#define PEEK_TX(index) tx_buffer[(tx_tail + (index)) & UART_TX_BUFFER_MASK]
// Internal methods
static char consume_rx(void);
static char consume_tx(void);
static void enqueue_rx(char c);
static void enqueue_tx(char c);
void uart_init(void)
{
// Clear buffers
memset(rx_buffer, 0, sizeof(rx_buffer));
rx_size = 0;
rx_head = 0;
rx_tail = 0;
memset(tx_buffer, 0, sizeof(tx_buffer));
tx_size = 0;
tx_head = 0;
tx_tail = 0;
// Enable & configure peripheral clocks
RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN; // Enable GPIO clock
RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; // Enable USART clock
RCC->CCIPR &= ~(RCC_CCIPR_USART2SEL); // Select system clock UART clock source
RCC->CCIPR |= (RCC_CCIPR_USART2SEL_0);
// Configure UART I/Os as AF
GPIOD->MODER &= 0xFFFFC3FF;
GPIOD->MODER |= 0x00002800;
GPIOD->AFR[0] |= 0x7700000;
// Configure USART2
USART2->CR1 &= ~(USART_CR1_UE | USART_CR1_M | USART_CR1_PCE | USART_CR1_OVER8);
USART2->CR2 &= ~USART_CR2_STOP;
USART2->BRR = 139; // 115200 baud
USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_TCIE);
USART2->GTPR |= 1; // Prescalar 1
USART2->CR1 |= USART_CR1_UE;
while((USART2->ISR & USART_ISR_TEACK) == 0);
while((USART2->ISR & USART_ISR_REACK) == 0);
// Enable USART2 ISR in NVIC
NVIC_EnableIRQ(USART2_IRQn);
}
uint32_t uart_getline(char* buffer, uint32_t buffer_len)
{
// Disable interrupts to prevent race conditions with ISR
uint32_t primask = __get_PRIMASK();
__disable_irq();
// Search for a newline (\n) in the receive buffer
uint32_t line_end = (uint32_t) -1; // Index into receive buffer
for (uint32_t i = 0; i < rx_size; i++)
{
if (PEEK_RX(i) == '\n')
{
line_end = i;
break;
}
}
if (line_end != (uint32_t) -1)
{
// Ignore a \r if present
if (PEEK_RX(line_end - 1) == '\r') line_end--;
// Make sure we don't overrun the destination buffer (including null terminator)
if ((line_end + 1) > buffer_len)
{
line_end = buffer_len - 1;
}
// Copy the line to the provided buffer (TODO could optimize this with memcpy)
for (uint32_t i = 0; i < line_end; i++)
{
buffer[i] = consume_rx();
}
// Add null terminator
buffer[line_end] = 0;
// Consume the newline
if (PEEK_RX(0) == '\r') consume_rx();
if (PEEK_RX(0) == '\n') consume_rx();
// Conditionally re-enable interrupts
if (!primask) __enable_irq();
// Return the number of bytes copied (including null terminator)
return line_end + 1;
}
else
{
// Conditionally re-enable interrupts
if (!primask) __enable_irq();
// If we didn't find a newline, return 0
return 0;
}
}
uint32_t uart_transmit(char* data, uint32_t len)
{
// Disable interrupts to prevent race conditions with ISR
uint32_t primask = __get_PRIMASK();
__disable_irq();
// Make sure we don't overrun the buffer
if (tx_size + len > UART_TX_BUFFER_SIZE)
{
len = UART_TX_BUFFER_SIZE - tx_size;
}
// Append the characters (TODO could optimize this with memcpy)
for (uint32_t i = 0; i < len; i++)
{
enqueue_tx(data[i]);
}
// Conditionally re-enable interrupts
if (!primask) __enable_irq();
// Return number of bytes enqueued
return len;
}
uint32_t uart_transmit_str(char* data)
{
uint32_t data_len = strlen(data);
return uart_transmit(data, data_len);
}
static char consume_rx()
{
if (rx_size > 0)
{
char c = rx_buffer[rx_tail];
rx_tail = (rx_tail + 1) & UART_RX_BUFFER_MASK;
rx_size--;
return c;
}
else
{
return 0;
}
}
static char consume_tx()
{
if (tx_size > 0)
{
char c = tx_buffer[tx_tail];
tx_tail = (tx_tail + 1) & UART_TX_BUFFER_MASK;
tx_size--;
return c;
}
else
{
return 0;
}
}
static void enqueue_rx(char c)
{
if (rx_size < UART_RX_BUFFER_SIZE)
{
rx_buffer[rx_head] = c;
rx_head = (rx_head + 1) & UART_RX_BUFFER_MASK;
rx_size++;
}
}
static void enqueue_tx(char c)
{
if (tx_size < UART_TX_BUFFER_SIZE)
{
uint32_t primask = __get_PRIMASK();
__disable_irq();
// If there's no ongoing transmit, start it now
if (tx_size == 0 && (USART2->ISR & USART_ISR_TXE))
{
USART2->TDR = c;
}
else // Otherwise buffer it
{
tx_buffer[tx_head] = c;
tx_head = (tx_head + 1) & UART_TX_BUFFER_MASK;
tx_size++;
}
if (primask) __enable_irq();
}
}
void USART2_IRQHandler(void)
{
uint32_t isr = USART2->ISR;
// Receive a character
if (isr & USART_ISR_RXNE)
{
enqueue_rx((char) USART2->RDR);
}
// Ready to transmit
if (isr & USART_ISR_TC)
{
// Clear flag
USART2->ICR |= USART_ICR_TCCF;
if (tx_size > 0)
{
USART2->TDR = consume_tx();
}
}
}