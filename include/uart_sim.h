#ifndef RISCSIMULATOR_S_UART_H
#define RISCSIMULATOR_S_UART_H

#include <stdint.h>
#include <ncurses.h>

#define UART_BASE_ADDR 0x10000000L

// UART Registers
#define RHR 0   // Receive Holding Register (read mode)
#define THR 0   // Transmit Holding Register (write mode)
#define DLL 0   // LSB of Divisor Latch (write mode)
#define IER 1   // Interrupt Enable Register (write mode)
#define DLM 1   // MSB of Divisor Latch (write mode)
#define FCR 2   // FIFO Control Register (write mode)
#define ISR 2   // Interrupt Status Register (read mode)
#define LCR 3   // Line Control Register
#define MCR 4   // Modem Control Register
#define LSR 5   // Line Status Register
#define MSR 6   // Modem Status Register
#define SPR 7   // ScratchPad Register

// Line Status Register bits
#define LSR_THRE 0x60 // Transmitter Holding Register Empty

typedef struct {
    uint8_t registers[8];
} UART;

void uart_init(UART *uart);
void uart_write(UART *uart, uint64_t addr, uint8_t value);
uint8_t uart_read(UART *uart, uint64_t addr);

#endif // RISCSIMULATOR_S_UART_H
