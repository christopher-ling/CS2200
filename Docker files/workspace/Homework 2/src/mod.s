!============================================================
! CS 2200 Homework 2 Part 1: mod
!
! Edit any part of this file necessary to implement the
! mod operation. Store your result in $v0.
!============================================================

mod:
    addi    $a0, $zero, 28      ! $a0 = 28, the number a to compute mod(a,b)
    addi    $a1, $zero, 13      ! $a1 = 13, the number b to compute mod(a,b)

    add    $t0, $zero, $a0      ! $t0 = a (x = a)
    add    $t1, $zero, $a1      ! $t1 = b
while:
    bgt    $t1, $t0, end         ! if b > x go to end
    nand $t2, $t1, $t1          ! negate b
    addi $t2, $t2, 1            ! -b
    add $t0, $t0, $t2           ! x = x - b
    beq $zero, $zero, while     ! back to start of while loop
end:
    add $v0, $t0, $zero
    halt

