.section .data
# 数据段，用于存储初始化的数据
test_data:
    .byte 0x12
    .half 0x3456
    .word 0x789ABCDE
    .dword 0x1122334455667788

.section .text
.global _start

_start:
    # 初始化寄存器
    li x1, 0            # x1 = 0
    la x2, test_data    # x2 = 数据段起始地址

    # 测试LB指令
    lb x3, 0(x2)        # x3 = *(int8_t *)(x2 + 0)

    # 测试LH指令
    lh x4, 1(x2)        # x4 = *(int16_t *)(x2 + 1)

    # 测试LW指令
    lw x5, 3(x2)        # x5 = *(int32_t *)(x2 + 3)

    # 测试LBU指令
    lbu x6, 0(x2)       # x6 = *(uint8_t *)(x2 + 0)

    # 测试LHU指令
    lhu x7, 1(x2)       # x7 = *(uint16_t *)(x2 + 1)

    # 测试SB指令
    li x8, 0xAABBCCDD
    sb x8, 4(x2)

    # 测试SH指令
    sh x8, 4(x2)

    # 测试SW指令
    sw x8, 4(x2)

    # 测试SD指令
    sd x8, 8(x2)

    # 结束
    li x10, 0xDEADBEEF