!==================================================================================
! CS 2200 Homework 2 Part 2: Fibonacci Sequence
!
! Apart from initializing the stack, please do not edit main's functionality.
! You do not need to save the return address before jumping to fibonacci in main.
!==================================================================================

main:
    add     $zero, $zero, $zero     ! TODO: Here, you need to get the address of the stack
                                    ! using the provided label to initialize the stack pointer.
                                    ! load the label address into $sp and in the next instruction,
                                    ! use $sp as base register to load the value (0xFFFF) into $sp.
                                    


    lea     $at, fibonacci          ! loads address of fibonacci label into $at

    lea     $a0, testFibInput1      ! loads address of number into $a0
    lw      $a0, 0($a0)             ! loads value of number into $a0

    jalr    $at, $ra                ! jump to fibonacci, set $ra to return addr
    halt                            ! when we return, just halt

fibonacci: 
    add     $zero, $zero, $zero     ! TODO: perform post-call portion of the calling convention.
                                    ! Make sure to save any registers you will be using!


    add     $zero, $zero, $zero     ! TODO: Implement the following pseudocode in assembly:
                                    ! IF ($a0 == 0)
                                    !    GOTO base (just returns $a0)
                                    ! ELSE IF ($a0 == 1)
                                    !    GOTO base 
                                    ! ELSE:
                                    !    GOTO else (main recursive logic)

else:
    add     $zero, $zero, $zero     ! TODO: perform recursion after decrementing
                                    ! the parameter by 1 (n - 1 step). Remember, 
                                    ! $a0 holds the parameter value. Then, 
                                    ! perform recursion for the second step after 
                                    ! decrementing the parameter by 1 again.
                                    ! Note that this process might be more involved
                                    ! than what it seems because of clobbering.
                                    ! (Hint: should we use the stack somewhere?)

    add     $zero, $zero, $zero     ! TODO: Implement the following pseudocode in assembly:
                                    ! $v0 = f(n - 1) + f(n - 2)
                                    ! RETURN $v0

base:
    add     $zero, $zero, $zero     ! TODO: Return $a0

teardown:
    add     $zero, $zero, $zero     ! TODO: perform pre-return portion
                                    ! of the calling convention
    
    jalr    $ra, $zero              ! return to caller


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