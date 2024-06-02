#ifndef RISCV_DEFS_H
#define RISCV_DEFS_H

// 指令类型宏定义
#define OPCODE_R_TYPE  0x33 // R-type指令，算术和逻辑操作
#define OPCODE_I_TYPE  0x13 // I-type指令，立即数操作和加载指令
#define OPCODE_S_TYPE  0x23 // S-type指令，存储指令
#define OPCODE_B_TYPE  0x63 // B-type指令，条件分支指令
#define OPCODE_U_TYPE  0x37 // U-type指令，上部立即数
#define OPCODE_J_TYPE  0x6F // J-type指令，无条件跳转

// 其他指令类型宏定义
#define OPCODE_LUI     0x37 // 加载上部立即数
#define OPCODE_AUIPC   0x17 // 加上立即数到程序计数器
#define OPCODE_JAL     0x6F // 跳转并链接
#define OPCODE_JALR    0x67 // 寄存器跳转并链接
#define OPCODE_BRANCH  0x63 // 条件分支
#define OPCODE_LOAD    0x03 // 加载指令
#define OPCODE_STORE   0x23 // 存储指令
#define OPCODE_IMM     0x13 // 立即数指令
#define OPCODE_OP      0x33 // 操作指令
#define OPCODE_MISC_MEM 0x0F // 杂项内存指令
#define OPCODE_SYSTEM  0x73 // 系统指令

// 指令字段掩码
#define OPCODE_MASK 0x7F
#define RD_MASK     0xF80
#define FUNCT3_MASK 0x7000
#define RS1_MASK    0xF8000
#define RS2_MASK    0x1F00000
#define FUNCT7_MASK 0xFE000000

// 宏用于提取指令字段
#define OPCODE(instruction) ((instruction) & OPCODE_MASK)
#define RD(instruction)     (((instruction) & RD_MASK) >> 7)
#define FUNCT3(instruction) (((instruction) & FUNCT3_MASK) >> 12)
#define RS1(instruction)    (((instruction) & RS1_MASK) >> 15)
#define RS2(instruction)    (((instruction) & RS2_MASK) >> 20)
#define FUNCT7(instruction) (((instruction) & FUNCT7_MASK) >> 25)

// R-type funct3 values
#define FUNCT3_ADD_SUB 0x0 // 加法/减法
#define FUNCT3_SLL     0x1 // 逻辑左移
#define FUNCT3_SLT     0x2 // 小于比较
#define FUNCT3_SLTU    0x3 // 无符号小于比较
#define FUNCT3_XOR     0x4 // 按位异或
#define FUNCT3_SRL_SRA 0x5 // 逻辑/算术右移
#define FUNCT3_OR      0x6 // 按位或
#define FUNCT3_AND     0x7 // 按位与

// R-type funct7 values
#define FUNCT7_ADD     0x00 // 加法
#define FUNCT7_SUB     0x20 // 减法
#define FUNCT7_SRL     0x00 // 逻辑右移
#define FUNCT7_SRA     0x20 // 算术右移

#endif // RISCV_DEFS_H
