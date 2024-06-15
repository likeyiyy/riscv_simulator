#ifndef RISCSIMULATOR_S_UART_H
#define RISCSIMULATOR_S_UART_H

#include <stdint.h>
#include <ncurses.h>

#define UART_BASE_ADDR 0x10000000L

// UART Registers
#define RHR_REG 0   // Receive Holding Register (read mode)
#define THR_REG 0   // Transmit Holding Register (write mode)
#define DLL_REG 0   // LSB of Divisor Latch (write mode)
#define IER_REG 1   // Interrupt Enable Register (write mode)
#define DLM_REG 1   // MSB of Divisor Latch (write mode)
#define FCR_REG 2   // FIFO Control Register (write mode)
#define ISR_REG 2   // Interrupt Status Register (read mode)
#define LCR_REG 3   // Line Control Register
#define MCR_REG 4   // Modem Control Register
#define LSR_REG 5   // Line Status Register
#define MSR_REG 6   // Modem Status Register
#define SPR_REG 7   // ScratchPad Register

// Line Status Register bits
#define LSR_RX_READY (1 << 0)
#define LSR_TX_IDLE  (1 << 5)
#define LSR_THRE 0x60 // Transmitter Holding Register Empty
#define UART_FIFO_SIZE 16  // FIFO 缓冲区大小



typedef struct {
    uint8_t THR;  // Transmitter Holding Register
    uint8_t RBR;  // Receiver Buffer Register
    uint8_t IER;  // Interrupt Enable Register
    uint8_t IIR;  // Interrupt Identification Register
    uint8_t FCR;  // FIFO Control Register
    uint8_t LCR;  // Line Control Register
    uint8_t MCR;  // Modem Control Register
    uint8_t LSR;  // Line Status Register
    uint8_t MSR;  // Modem Status Register
    uint8_t SCR;  // Scratch Register
    uint16_t DLL; // Divisor Latch Low
    uint16_t DLM; // Divisor Latch High
    uint8_t fifo[UART_FIFO_SIZE];  // 接收 FIFO 缓冲区
    uint8_t fifo_head;  // FIFO 头指针
    uint8_t fifo_tail;  // FIFO 尾指针
    uint8_t fifo_count; // FIFO 中的字节数
} UART;


UART* get_uart(void);
void uart_init(UART *uart);
uint64_t uart_read(uint64_t addr, uint32_t size);
void uart_write(uint64_t address, uint64_t value, uint32_t size);


#endif // RISCSIMULATOR_S_UART_H
