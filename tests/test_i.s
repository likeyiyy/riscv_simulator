.global _start
.text

_start:
    # 初始化寄存器
    li x2, 5         # 将立即数 5 加载到 x2 寄存器中 (ADDI)
    li x3, 10        # 将立即数 10 加载到 x3 寄存器中 (ADDI)

    # 测试 R-type 指令
    add x4, x2, x3   # x4 = x2 + x3 = 5 + 10 = 15
    sub x5, x3, x2   # x5 = x3 - x2 = 10 - 5 = 5

    # 测试 I-type 指令
    addi x6, x2, 3   # x6 = x2 + 3 = 5 + 3 = 8
    slli x7, x2, 2   # x7 = x2 << 2 = 5 << 2 = 20
    slti x8, x2, 6   # x8 = (x2 < 6) ? 1 : 0 = (5 < 6) ? 1 : 0 = 1
    sltiu x9, x2, 6  # x9 = (x2 < 6) ? 1 : 0 = (5 < 6) ? 1 : 0 = 1
    xori x10, x2, 1  # x10 = x2 ^ 1 = 5 ^ 1 = 4
    srli x11, x2, 1  # x11 = x2 >> 1 = 5 >> 1 = 2
    srai x12, x2, 1  # x12 = x2 >> 1 = 5 >> 1 = 2 (算术右移)
    ori x13, x2, 2   # x13 = x2 | 2 = 5 | 2 = 7
    andi x14, x2, 3  # x14 = x2 & 3 = 5 & 3 = 1

