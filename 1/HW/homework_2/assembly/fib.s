!==================================================================================
! CS 2200 Homework 2 Part 2: Fibonacci Sequence
!
! Apart from initializing the stack, please do not edit main's functionality.
! You do not need to save the return address before jumping to fibonacci in main.
!==================================================================================

main:
    lea     $sp, stack              ! Initialize stack pointer
    lw      $sp, 0($sp)             ! Load stack address into $sp

    lea     $at, fibonacci          ! Load address of fibonacci into $at
    lea     $a0, testFibInput1      ! Load address of test input into $a0
    lw      $a0, 0($a0)             ! Load value of test input into $a0

    jalr    $at, $ra                ! Jump to fibonacci, set $ra to return address
    halt                            ! Halt after returning from fibonacci

fibonacci:
    ! === Prologue: Save registers and set up stack frame ===
    addi    $sp, $sp, -8            ! Allocate space on the stack for $ra and $fp
    sw      $ra, 4($sp)             ! Save return address ($ra) on the stack
    sw      $fp, 0($sp)             ! Save frame pointer ($fp) on the stack
    addi    $fp, $sp, 8             ! Set frame pointer to the base of the stack frame

    ! === Base Cases ===
    beq     $a0, $zero, base_case_0 ! If n == 0, jump to base_case_0
    addi    $t0, $zero, 1           ! Load 1 into $t0
    beq     $a0, $t0, base_case_1   ! If n == 1, jump to base_case_1

    ! === Recursive Case ===
    ! Save $a0 (n) on the stack
    addi    $sp, $sp, -4            ! Allocate space for $a0
    sw      $a0, 0($sp)             ! Save $a0 (n) on the stack

    ! Call fibonacci(n - 1)
    addi    $a0, $a0, -1            ! Decrement n by 1
    jalr    $at, $ra                ! Recursive call: fibonacci(n - 1)
    addi    $sp, $sp, -4            ! Allocate space for the result of fibonacci(n - 1)
    sw      $v0, 0($sp)             ! Save the result of fibonacci(n - 1) on the stack

    ! Call fibonacci(n - 2)
    lw      $a0, 4($sp)             ! Restore original n from the stack
    addi    $a0, $a0, -2            ! Decrement n by 2
    jalr    $at, $ra                ! Recursive call: fibonacci(n - 2)

    ! Add the results of fibonacci(n - 1) and fibonacci(n - 2)
    lw      $t0, 0($sp)             ! Load the result of fibonacci(n - 1) from the stack
    add     $v0, $v0, $t0           ! Add the results: $v0 = fibonacci(n - 1) + fibonacci(n - 2)

    ! Clean up the stack
    addi    $sp, $sp, 8             ! Deallocate space for $a0 and the result of fibonacci(n - 1)

    ! === Epilogue: Restore registers and return ===
    beq     $zero, $zero, teardown  ! Unconditional branch to teardown (emulate j teardown)

base_case_0:
    add     $v0, $zero, $zero       ! Return 0
    beq     $zero, $zero, teardown  ! Unconditional branch to teardown (emulate j teardown)

base_case_1:
    addi    $v0, $zero, 1           ! Return 1
    beq     $zero, $zero, teardown  ! Unconditional branch to teardown (emulate j teardown)

teardown:
    ! === Restore registers and return ===
    lw      $fp, 0($sp)             ! Restore frame pointer ($fp) from the stack
    lw      $ra, 4($sp)             ! Restore return address ($ra) from the stack
    addi    $sp, $sp, 8             ! Deallocate space for $fp and $ra
    jalr    $ra, $zero              ! Return to caller

stack: .word 0xFFFF                 ! the stack begins here


! Words for testing \/

! 1
testFibInput1:
    .word 0x0001

! 10
testFibInput2:
    .word 0x000a

! 20
testFibInput3:
    .word 0x0014