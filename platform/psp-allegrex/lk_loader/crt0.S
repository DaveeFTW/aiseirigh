/*
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

.set noreorder

# there are a variety of factors that could lead to execution here. during cold
# boot the system will have the rom mapped at 0xBFC00000 and the RWX exception
# vector memory location mapped at 0xBFD00000. when reset, this vector memory is
# remapped to the 0xBFC00000.
# In the situation where we have the reset exploit used to execute this code,
# then there is no reset to be performed. the code below checks for some markers
# and then skips the remapping process.
# this allows such an IPL to be used as-is and as a payload compatible with the
# reset exploit.
.section ".text.boot"
.global _start
_start:
    la $t8, _start
    bne $t8, $t9, reset_cpu
    li $v1, 0xDAEE
    bne $v0, $v1, reset_cpu
    nop
    j from_reset
    nop

reset_cpu:
    la $t0, 0xBFD00000
    la $t2, from_reset
    lui $t1, 0x3C08
    srl $t3, $t2, 16
    or $t1, $t3
    andi $t3, $t2, 0xFFFF
    lui $t2, 0x3508
    or $t2, $t3
    li $t3, 0x1000008
    sw $t1, 0($t0)
    sw $t2, 4($t0)
    sw $t3, 8($t0)
    sw $0, 12($t0)
    sync
    la $a0, 0xBC10004C
    lw $t0, 0($a0)
    ori $t0, 2
    sw $t0, 0($a0)

sync_loop:
    b sync_loop
    sync

from_reset:
    lui $v0, 0x6000
    mtc0 $v0, $12
    mtc0 $0, $13
    nop
    li $t1, 0xFFFFFFFF
    lui $t0, 0xBC10
    sw $t1, 4($t0)

from_reset2:
    la $sp, 0x041FFFF0
    la $a0, __bss_start
    la $a1, __bss_end

clear_bss:
    sltu $a2, $a0, $a1
    addiu $a0, $a0, 4
    bnel $a2, $0, clear_bss
    sw $0, -4($a0)

    jal main
    nop

inf_loop:
    b inf_loop
    nop
