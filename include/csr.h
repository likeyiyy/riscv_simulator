//
// Created by gllue new on 2024/6/8.
//

#ifndef RISCSIMULATOR_CSR_H
#define RISCSIMULATOR_CSR_H

// 用户模式 CSRs
#define CSR_USTATUS     0x000
#define CSR_UIE         0x004
#define CSR_UTVEC       0x005
#define CSR_USCRATCH    0x040
#define CSR_UEPC        0x041
#define CSR_UCAUSE      0x042
#define CSR_UTVAL       0x043
#define CSR_UIP         0x044

// 特权级寄存器的 CSR
#define CSR_FFLAGS      0x001
#define CSR_FRM         0x002
#define CSR_FCSR        0x003

// 超级模式 CSRs
#define CSR_SSTATUS     0x100
#define CSR_SEDELEG     0x102
#define CSR_SIDELEG     0x103
#define CSR_SIE         0x104
#define CSR_STVEC       0x105
#define CSR_SCOUNTEREN  0x106

#define CSR_SSCRATCH    0x140
#define CSR_SEPC        0x141
#define CSR_SCAUSE      0x142
#define CSR_STVAL       0x143
#define CSR_SIP         0x144
#define CSR_SATP        0x180

// 机器模式 CSRs
#define CSR_MVENDORID   0xF11
#define CSR_MARCHID     0xF12
#define CSR_MIMPID      0xF13
#define CSR_MHARTID     0xF14

#define CSR_MSTATUS     0x300
#define CSR_MISA        0x301
#define CSR_MEDELEG     0x302
#define CSR_MIDELEG     0x303
#define CSR_MIE         0x304
#define CSR_MTVEC       0x305
#define CSR_MCOUNTEREN  0x306

#define CSR_MSCRATCH    0x340
#define CSR_MEPC        0x341
#define CSR_MCAUSE      0x342
#define CSR_MTVAL       0x343
#define CSR_MIP         0x344

// 机器模式性能计数器 CSRs
#define CSR_MCYCLE      0xB00
#define CSR_MINSTRET    0xB02
#define CSR_MCYCLEH     0xB80
#define CSR_MINSTRETH   0xB82

// 虚拟化相关 CSRs（如果实现支持虚拟化）
#define CSR_VSSTATUS    0x200
#define CSR_VSIE        0x204
#define CSR_VSTVEC      0x205
#define CSR_VSSCRATCH   0x240
#define CSR_VSEPC       0x241
#define CSR_VSCAUSE     0x242
#define CSR_VSTVAL      0x243
#define CSR_VSIP        0x244

// 其他 CSRs
#define CSR_TSELECT     0x7A0
#define CSR_TDATA1      0x7A1
#define CSR_TDATA2      0x7A2
#define CSR_TDATA3      0x7A3

// 调试 CSRs
#define CSR_DCSR        0x7B0
#define CSR_DPC         0x7B1
#define CSR_DSCRATCH    0x7B2

#define PRV_U 0 // User
#define PRV_S 1 // Supervisor
#define PRV_M 3 // Machine

// ECALL 异常代码
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_USER_ECALL       8
#define CAUSE_SUPERVISOR_ECALL 9
#define CAUSE_HYPERVISOR_ECALL 10
#define CAUSE_MACHINE_ECALL    11


#endif //RISCSIMULATOR_CSR_H
