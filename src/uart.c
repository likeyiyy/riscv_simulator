#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "mfprintf.h"

static UART global_uart;
UART* get_uart(void) {
    return &global_uart;
}

void uart_init(UART *uart) {
    memset(uart->registers, 0, sizeof(uart->registers));
    uart->registers[LSR] = LSR_THRE; // Initialize LSR with THR empty
}

void uart_write(uint64_t offset, uint64_t value, uint32_t size) {
    UART *uart = get_uart();

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

uint64_t uart_read(uint64_t offset, uint32_t size) {
    UART *uart = get_uart();
    if (offset < sizeof(uart->registers)) {
        return uart->registers[offset];
    }
    return 0;
}
