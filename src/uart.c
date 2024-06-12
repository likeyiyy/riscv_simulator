#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "mfprintf.h"

static UART global_uart;
UART* get_uart(void) {
    return &global_uart;
}

void uart_init(UART *uart) {
    uart->THR = 0;
    uart->RBR = 0;
    uart->IER = 0;
    uart->IIR = 0x01; // No interrupt pending
    uart->FCR = 0;
    uart->LCR = 0;
    uart->MCR = 0;
    uart->LSR = 0x60; // Transmitter empty and idle
    uart->MSR = 0;
    uart->SCR = 0;
    uart->DLL = 0;
    uart->DLM = 0;
}


void _uart_write(UART *uart, uint16_t port, uint8_t value) {
    switch (port) {
        case 0: uart->THR = value; break;
        case 1: uart->IER = value; break;
        case 2: uart->FCR = value; break;
        case 3: uart->LCR = value; break;
        case 4: uart->MCR = value; break;
        case 5: uart->LSR = value; break;
        case 6: uart->MSR = value; break;
        case 7: uart->SCR = value; break;
        case 8: uart->DLL = (uart->DLL & 0xFF00) | value; break;
        case 9: uart->DLL = (uart->DLL & 0x00FF) | (value << 8); break;
    }
}

void uart_write(uint64_t address, uint64_t value, uint32_t size) {
    UART *uart = get_uart();
    uint64_t offset = address - UART_BASE_ADDR;
    uint64_t port = offset;
    if (offset < 8) {
        if (uart->LCR & 0x80 && (offset == DLL_REG || offset == DLM_REG)) {
	    port = 0x8 + offset;
        } else if (offset == THR_REG) {
            uart->LSR &= ~LSR_THRE; // Clear THR empty bit
    	}
	_uart_write(uart, port, value);
    }
}

uint8_t _uart_read(UART *uart, uint16_t port) {
    switch (port) {
        case 0: return uart->RBR;
        case 1: return uart->IER;
        case 2: return uart->IIR;
        case 3: return uart->LCR;
        case 4: return uart->MCR;
        case 5: return uart->LSR;
        case 6: return uart->MSR;
        case 7: return uart->SCR;
        case 8: return uart->DLL & 0xFF;
        case 9: return (uart->DLL >> 8) & 0xFF;
        default: return 0;
    }
}

uint64_t uart_read(uint64_t addr, uint32_t size) {
    UART *uart = get_uart();
    uint64_t offset = addr - UART_BASE_ADDR;
    uint64_t port = offset;
    if (offset < 8) {
        if (uart->LCR & 0x80 && (offset == DLL_REG || offset == DLM_REG)) {
	    port = 0x8 + offset;
        }
        char value = _uart_read(uart, port);
	if (((uart->LCR & 0x80) == 0) && offset == RHR_REG) {
            uart->LSR &= ~LSR_RX_READY; // Set Data NOT Ready
	}
	return value;
    }
    return 0;
}
