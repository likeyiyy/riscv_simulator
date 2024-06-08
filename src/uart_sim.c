#include <stdint.h>
#include <string.h>
#include "uart_sim.h"

void uart_init(UART *uart) {
    memset(uart->registers, 0, sizeof(uart->registers));
    uart->registers[LSR] = LSR_THRE; // Initialize LSR with THR empty
}

void uart_write(UART *uart, uint64_t addr, uint8_t value) {
    uint64_t offset = addr - UART_BASE_ADDR;

    if (offset < sizeof(uart->registers)) {
        if (uart->registers[LCR] & 0x80 && (offset == DLL || offset == DLM)) {
            return;
        }
        uart->registers[(uint8_t)offset] = value;
    }

    if (offset == THR) {
        uart->registers[LSR] &= ~LSR_THRE; // Clear THR empty bit
        uart->registers[LSR] |= 0x01; // Set Data Ready
    }
}

uint8_t uart_read(UART *uart, uint64_t addr) {
    uint64_t offset = addr - UART_BASE_ADDR;
    if (offset < sizeof(uart->registers)) {
        return uart->registers[offset];
    }
    return 0;
}
