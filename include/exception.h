#ifndef RISCV_SIMULATOR_EXCEPTION_H
#define RISCV_SIMULATOR_EXCEPTION_H
#include "cpu.h"

void raise_exception(CPU *cpu, uint64_t cause);
bool handle_interrupt(CPU *cpu);


#endif //RISCV_SIMULATOR_EXCEPTION_H
