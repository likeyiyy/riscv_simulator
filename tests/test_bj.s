# test_bj.s - 测试 B 型和 J 型指令

.section .data
value1:  .word 1
value2:  .word 2

.section .text
.globl _start
_start:
    # 测试 BEQ
    li x1, 1              # 将立即数 1 加载到寄存器 x1
    li x2, 1              # 将立即数 1 加载到寄存器 x2
    beq x1, x2, equal     # 如果 x1 == x2，跳转到标签 equal

    # 测试 BNE
    li x1, 1              # 将立即数 1 加载到寄存器 x1
    li x2, 2              # 将立即数 2 加载到寄存器 x2
    bne x1, x2, not_equal # 如果 x1 != x2，跳转到标签 not_equal

    # 测试 BLT
    li x1, 1              # 将立即数 1 加载到寄存器 x1
    li x2, 2              # 将立即数 2 加载到寄存器 x2
    blt x1, x2, less_than # 如果 x1 < x2，跳转到标签 less_than

    # 测试 BGE
    li x1, 2              # 将立即数 2 加载到寄存器 x1
    li x2, 1              # 将立即数 1 加载到寄存器 x2
    bge x1, x2, greater_equal # 如果 x1 >= x2，跳转到标签 greater_equal

    # 测试 JAL
    jal x3, jump_label    # 无条件跳转到 jump_label，并将返回地址存储在 x3 中

    # 测试 JALR
    la x4, jump_label     # 将 jump_label 的地址加载到 x4 中
    jalr x5, 0(x4)        # 跳转到 x4 所指向的地址，并将返回地址存储在 x5 中

    j end                 # 跳转到结束标签，防止执行到未定义的指令

equal:
    li x6, 1              # 如果跳转到这里，表示 BEQ 测试成功
    j end

not_equal:
    li x7, 1              # 如果跳转到这里，表示 BNE 测试成功
    j end

less_than:
    li x8, 1              # 如果跳转到这里，表示 BLT 测试成功
    j end

greater_equal:
    li x9, 1              # 如果跳转到这里，表示 BGE 测试成功
    j end

jump_label:
    li x10, 1             # 如果跳转到这里，表示 JAL 测试成功
    j end

end:
    nop                   # 结束程序

