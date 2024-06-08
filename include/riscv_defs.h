#ifndef RISCV_DEFS_H
#define RISCV_DEFS_H

// 指令类型宏定义
// Load instructions: Load data from memory to register
#define OPCODE_LOAD      0x03  // Load: LB, LH, LW, LBU, LHU, LD

// Load-FP instructions: Load floating-point data from memory to FP register
#define OPCODE_LOAD_FP   0x07  // Load Floating-Point: FLW, FLD

// Custom-0 instructions: Custom extension opcode 0
#define OPCODE_CUSTOM_0 0x0B  // Custom-0 (user-defined)

#define OPCODE_MISC_MEM  0x0F  // Miscellaneous Memory: FENCE, FENCE.I
#define OPCODE_OP_IMM    0x13  // OP-IMM: ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI
#define OPCODE_AUIPC     0x17  // Add Upper Immediate to PC: AUIPC
#define OPCODE_OP_IMM_32 0x1B  // Arithmetic Immediate (32-bit): ADDIW, SLLIW, SRLIW, SRAIW
#define OPCODE_STORE     0x23  // Store: SB, SH, SW, SD
#define OPCODE_STORE_FP  0x27  // Store Floating-Point: FSW, FSD

// Custom-1 instructions: Custom extension opcode 1
#define OPCODE_CUSTOM_1 0x2B  // Custom-1 (user-defined)

#define OPCODE_AMO       0x2F  // Atomic Memory Operations: LR.W, SC.W, AMOSWAP.W, AMOADD.W, AMOXOR.W, AMOAND.W, AMOOR.W, AMOMIN.W, AMOMAX.W, AMOMINU.W, AMOMAXU.W, LR.D, SC.D, AMOSWAP.D, AMOADD.D, AMOXOR.D, AMOAND.D, AMOOR.D, AMOMIN.D, AMOMAX.D, AMOMINU.D, AMOMAXU.D
#define OPCODE_OP        0x33  // OP: ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
#define OPCODE_LUI       0x37  // Load Upper Immediate: LUI
#define OPCODE_OP_32     0x3B  // Arithmetic (32-bit): ADDW, SUBW, SLLW, SRLW, SRAW

// Floating-Point Instructions (if implemented)
#define OPCODE_MADD      0x43  // Multiply-Add: FMADD.S, FMADD.D
#define OPCODE_MSUB      0x47  // Multiply-Subtract: FMSUB.S, FMSUB.D
#define OPCODE_NMSUB     0x4B  // Negative Multiply-Subtract: FNMSUB.S, FNMSUB.D
#define OPCODE_NMADD     0x4F  // Negative Multiply-Add: FNMADD.S, FNMADD.D
#define OPCODE_OP_FP     0x53  // Floating-Point Operations: FADD.S, FSUB.S, FMUL.S, FDIV.S, FSQRT.S, FSGNJ.S, FSGNJN.S, FSGNJX.S, FMIN.S, FMAX.S, FCVT.W.S, FCVT.WU.S, FMV.X.W, FEQ.S, FLT.S, FLE.S, FCLASS.S, FCVT.S.W, FCVT.S.WU, FMV.W.X, FCVT.L.S, FCVT.LU.S, FCVT.S.L, FCVT.S.LU, FMV.X.D, FSGNJ.D, FSGNJN.D, FSGNJX.D, FMIN.D, FMAX.D, FCVT.W.D, FCVT.WU.D, FMV.X.D, FEQ.D, FLT.D, FLE.D, FCLASS.D, FCVT.D.W, FCVT.D.WU, FMV.D.X

// Custom-2/rv128 instructions: Custom extension opcode 2 or RV128 instructions
#define OPCODE_CUSTOM_2 0x5B  // Custom-2 (user-defined)

#define OPCODE_BRANCH    0x63  // Branch: BEQ, BNE, BLT, BGE, BLTU, BGEU
#define OPCODE_JALR      0x67  // Jump and Link Register: JALR
#define OPCODE_JAL       0x6F  // Jump and Link: JAL
#define OPCODE_SYSTEM    0x73  // System: ECALL, EBREAK, CSRRW, CSRRS, CSRRC, CSRRWI, CSRRSI, CSRRCI

// Custom-3/rv128 instructions: Custom extension opcode 3 or RV128 instructions
#define OPCODE_CUSTOM_3 0x7B  // Custom-3 (user-defined)


#define FUNCT3_ADDIW 0x0  // ADDIW指令的功能码
#define FUNCT3_SLLIW 0x1  // SLLIW指令的功能码
#define FUNCT3_SRLIW 0x5  // SRLIW指令的功能码
#define FUNCT3_SRAIW 0x5  // SRAIW指令的功能码

#define FUNCT3_ADDW 0x0   // ADDW指令的功能码
#define FUNCT3_SUBW 0x0   // SUBW指令的功能码
#define FUNCT3_SLLW 0x1   // SLLW指令的功能码
#define FUNCT3_SRLW 0x5   // SRLW指令的功能码
#define FUNCT3_SRAW 0x5   // SRAW指令的功能码


// 指令字段掩码
#define OPCODE_MASK 0x7F
#define RD_MASK     0xF80
#define FUNCT3_MASK 0x7000
#define RS1_MASK    0xF8000
#define RS2_MASK    0x1F00000
#define FUNCT7_MASK 0xFE000000
#define IMM_MASK 0xFFF00000


// 宏用于提取指令字段
#define OPCODE(instruction) ((instruction) & OPCODE_MASK)
#define RD(instruction)     (((instruction) & RD_MASK) >> 7)
#define FUNCT3(instruction) (((instruction) & FUNCT3_MASK) >> 12)
#define RS1(instruction)    (((instruction) & RS1_MASK) >> 15)
#define RS2(instruction)    (((instruction) & RS2_MASK) >> 20)
#define FUNCT7(instruction) (((instruction) & FUNCT7_MASK) >> 25)
#define IMM(instruction) ((int32_t)((instruction) >> 20))

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

// I 型指令的 funct3 定义
#define FUNCT3_ADDI 0x0      // 加法立即数
#define FUNCT3_SLLI 0x1      // 逻辑左移立即数
#define FUNCT3_SLTI 0x2      // 小于比较立即数
#define FUNCT3_SLTIU 0x3     // 无符号小于比较立即数
#define FUNCT3_XORI 0x4      // 按位异或立即数
#define FUNCT3_SRLI_SRAI 0x5 // 逻辑右移立即数或算术右移立即数
#define FUNCT3_ORI 0x6       // 按位或立即数
#define FUNCT3_ANDI 0x7      // 按位与立即数


// SRAI 的 funct7 定义
#define FUNCT7_SRLI 0x00     // 逻辑右移立即数
#define FUNCT7_SRAI 0x20     // 算术右移立即数


#define FUNCT3_SB 0x0 // 存储字节
#define FUNCT3_SH 0x1 // 存储半字
#define FUNCT3_SW 0x2 // 存储字
#define FUNCT3_SD 0x3 // 存储双字

#define FUNCT3_LB  0x0 // 加载字节
#define FUNCT3_LH  0x1 // 加载半字
#define FUNCT3_LW  0x2 // 加载字
#define FUNCT3_LD  0x3 // 加载双字
#define FUNCT3_LBU 0x4 // 加载字节无符号
#define FUNCT3_LHU 0x5 // 加载半字无符号


// B 型指令的 funct3 值
#define FUNCT3_BEQ  0x0
#define FUNCT3_BNE  0x1
#define FUNCT3_BLT  0x4
#define FUNCT3_BGE  0x5
#define FUNCT3_BLTU 0x6
#define FUNCT3_BGEU 0x7

// funct3 for LR/SC instructions
#define FUNCT3_LR 0x2
#define FUNCT3_SC 0x3

// funct7 for AMO instructions
#define FUNCT7_AMOSWAP 0x01
#define FUNCT7_AMOADD 0x00
#define FUNCT7_AMOXOR 0x04
#define FUNCT7_AMOAND 0x0C
#define FUNCT7_AMOOR 0x08
#define FUNCT7_AMOMIN 0x10
#define FUNCT7_AMOMAX 0x14
#define FUNCT7_AMOMINU 0x18
#define FUNCT7_AMOMAXU 0x1C

// M 扩展指令的 funct3 和 funct7
#define FUNCT7_M 0x01
#define FUNCT3_MUL    0x0
#define FUNCT3_MULH   0x1
#define FUNCT3_MULHSU 0x2
#define FUNCT3_MULHU  0x3
#define FUNCT3_DIV    0x4
#define FUNCT3_DIVU   0x5
#define FUNCT3_REM    0x6
#define FUNCT3_REMU   0x7

#endif // RISCV_DEFS_H
