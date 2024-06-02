#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#include <stdint.h>

// 定义寄存器名称
extern const char* reg_names[];

// 反汇编函数
void disassemble(uint32_t instruction, char* buffer, size_t buffer_size);


#endif // DISASSEMBLE_H
