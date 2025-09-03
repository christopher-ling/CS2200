! This program executes pow as a test program using the LC-3200a calling convention
! Check your registers ($v0) and memory to see if it is consistent with this program

! vector table
vector0:
        .fill 0x00000000                        ! device ID 0
        .fill 0x00000000                        ! device ID 1
        .fill 0x00000000                        ! ...
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000                        ! device ID 7
        ! end vector table

main:	lea $sp, initsp                         ! initialize the stack pointer
        lw $sp, 0($sp)                          ! finish initialization


        ! TO-DO #1 ========================================================================================================
        ! Load the address of the timer_handler into the IVT using vector0
        ! =================================================================================================================
        lea $t0, vector0
        lea $t1, timer_handler
        sw $t1, 0($t0)


        ! TO-DO #2 ========================================================================================================
        ! Load the address of the distance_tracker_handler into the IVT (vector0 was already loaded in to-do #1)
        ! =================================================================================================================
        lea $t1, distance_tracker_handler
        sw $t1, 1($t0)


        lea $t0, minval
        lw $t0, 0($t0)
	lea $t1, INT_MAX 			! store 0x7FFFFFFF into minval (to initialize)
	lw $t1, 0($t1)	                  		
        sw $t1, 0($t0)

        ei                                      ! Enable interrupts

        addi $t0, $zero, 5                      ! Code to test whether tern/cols/swap still work
        addi $t1, $zero, -15
        tern $t2, $t0, $t1
        cols $t2, $t1, $t0
        swap $t0, $t1

        add $t0, $zero, $zero
        add $t1, $zero, $zero
        add $t2, $zero, $zero

        lea $a0, BASE                           ! load base for pow
        lw $a0, 0($a0)
        lea $a1, EXP                            ! load power for pow
        lw $a1, 0($a1)
        lea $at, POW                            ! load address of pow
        jalr $at, $ra                           ! run pow
        lea $a0, ANS                            ! load base for pow
        sw $v0, 0($a0)

        halt                                    ! stop the program here
        addi $v0, $zero, -1                     ! load a bad value on failure to halt

BASE:   .fill 2
EXP:    .fill 8
ANS:	.fill 0                                 ! should come out to 256 (BASE^EXP)

INT_MAX: .fill 0x7FFFFFFF

POW:    addi $sp, $sp, -1                       ! allocate space for old frame pointer
        sw $fp, 0($sp)

        addi $fp, $sp, 0                        ! set new frame pointer

        bgt $a1, $zero, BASECHK                 ! check if $a1 is zero
        beq $zero, $zero, RET1                  ! if the exponent is 0, return 1

BASECHK:bgt $a0, $zero, WORK                    ! if the base is 0, return 0
        beq $zero, $zero, RET0

WORK:   addi $a1, $a1, -1                       ! decrement the power
        lea $at, POW                            ! load the address of POW
        addi $sp, $sp, -2                       ! push 2 slots onto the stack
        sw $ra, -1($fp)                         ! save RA to stack
        sw $a0, -2($fp)                         ! save arg 0 to stack
        jalr $at, $ra                           ! recursively call POW
        add $a1, $v0, $zero                     ! store return value in arg 1
        lw $a0, -2($fp)                         ! load the base into arg 0
        lea $at, MULT                           ! load the address of MULT
        jalr $at, $ra                           ! multiply arg 0 (base) and arg 1 (running product)
        lw $ra, -1($fp)                         ! load RA from the stack
        addi $sp, $sp, 2

        beq $zero, $zero, FIN                   ! unconditional branch to FIN

RET1:   add $v0, $zero, $zero                   ! return a value of 0
	addi $v0, $v0, 1                        ! increment and return 1
        beq $zero, $zero, FIN                   ! unconditional branch to FIN

RET0:   add $v0, $zero, $zero                   ! return a value of 0

FIN:	lw $fp, 0($fp)                          ! restore old frame pointer
        addi $sp, $sp, 1                        ! pop off the stack
        jalr $ra, $zero

MULT:   add $v0, $zero, $zero                   ! return value = 0
        addi $t0, $zero, 0                      ! sentinel = 0
AGAIN:  add $v0, $v0, $a0                       ! return value += argument0
        addi $t0, $t0, 1                        ! increment sentinel
        bgt $a1, $t0, AGAIN                     ! while sentinel < argument, loop again
        jalr $ra, $zero                         ! return from mult


timer_handler:

        ! TO-DO #3 ========================================================================================================
        ! Implement the timer_hander code by first doing handler setup (save $k0, enable interrupts, then save registers).
        !
        ! Next, retrieve ticks from memory, increment it by 1, then restore it back into memory
        !
        ! Finally, do the handler "teardown" (restore processor state, disable interrupts, then RETI). 
        ! =================================================================================================================
        ! Buildup
        addi $sp, $sp, -1       !Save $k0 in stack
        sw $k0, 0($sp)

        ei                      ! Enable interrupts

        addi $sp, $sp, -1       ! Save registers
        sw $t0, 0($sp)
        addi $sp, $sp, -1
        sw $t1, 0($sp)
        addi $sp, $sp, -1
        sw $t2, 0($sp)
        addi $sp, $sp, -1
        sw $s0, 0($sp)
        addi $sp, $sp, -1
        sw $s1, 0($sp)
        addi $sp, $sp, -1
        sw $s2, 0($sp)

        ! Timer
        lea $t0, ticks          ! Retrieve ticks from memory
        lw $t0, 0($t0)          ! Address
        lw $t1, 0($t0)          ! Value
        addi $t1, $t1, 1        ! Increment
        sw $t1, 0($t0)          ! Store into memory

        ! Teardown
        lw $s2, 0($sp)          ! Restore registers
        addi $sp, $sp, 1
        lw $s1, 0($sp)
        addi $sp, $sp, 1
        lw $s0, 0($sp)
        addi $sp, $sp, 1
        lw $t2, 0($sp)
        addi $sp, $sp, 1
        lw $t1, 0($sp)
        addi $sp, $sp, 1
        lw $t0, 0($sp)
        addi $sp, $sp, 1

        di                      ! Disable interrupts

        lw $k0, 0($sp)          ! Restore $k0
        addi $sp, $sp, 1

        reti


distance_tracker_handler:

        ! TO-DO #4 ========================================================================================================
        ! Implement the distance_tracker_handler code by first doing handler setup (save $k0, enable interrupts, then save registers).
        !
        ! Then, retrieve the current val from the distance tracker, update maxval and minval accordingly, and then calculate the range
        !
        ! Finally, do the handler "teardown" (restore processor state, disable interrupts, then RETI).
        ! =================================================================================================================
        ! Buildup
        addi $sp, $sp, -1               !Save $k0 in stack
        sw $k0, 0($sp)

        ei                              ! Enable interrupts

        addi $sp, $sp, -1               ! Save registers
        sw $t0, 0($sp)
        addi $sp, $sp, -1
        sw $t1, 0($sp)
        addi $sp, $sp, -1
        sw $t2, 0($sp)
        addi $sp, $sp, -1
        sw $s0, 0($sp)
        addi $sp, $sp, -1
        sw $s1, 0($sp)
        addi $sp, $sp, -1
        sw $s2, 0($sp)

        ! Tracker
        in $t0, 1                       ! Value of distance tracker

        lea $t1, maxval
        lw $t1, 0($t1)                  ! Address of maxval in $t1
        lw $s0, 0($t1)                  ! Current value of maxval in $s0

        lea $t2, minval
        lw $t2, 0($t2)                  ! Address of minval in $t2
        lw $s1, 0($t2)                  ! Current value of minval in $s1

        bgt $t0, $s0, UPDATE_MAXVAL     ! Check if current is greater than maxval of distance tracker
        beq $zero, $zero, MIN_CHECK

        UPDATE_MAXVAL:
        sw $t0, 0($t1)                  ! Store current val into address of maxval
        add $s0, $t0, $zero             ! Updates current value of maxval in $s0 to current val from distance tracker

        MIN_CHECK:
        bgt $s1, $t0, UPDATE_MINVAL     ! Check if minval is greater than current val of distance tracker
        beq $zero, $zero, RANGE

        UPDATE_MINVAL:
        sw $t0, 0($t2)                  ! Store current val into address of minval
        add $s1, $t0, $zero

        RANGE:
        nand $s1, $s1, $s1
        addi $s1, $s1, 1                ! Negate minval

        add $s2, $s0, $s1               ! Calculate range ($t2 = maxval - minval)

        lea $t0, range                  ! Load effective address of range into $t1
        lw $t0, 0($t0)                  ! Address of range
        sw $s2, 0($t0)                  ! Store range in memory

        DONE:
        ! Teardown
        lw $s2, 0($sp)          ! Restore registers
        addi $sp, $sp, 1
        lw $s1, 0($sp)
        addi $sp, $sp, 1
        lw $s0, 0($sp)
        addi $sp, $sp, 1
        lw $t2, 0($sp)
        addi $sp, $sp, 1
        lw $t1, 0($sp)
        addi $sp, $sp, 1
        lw $t0, 0($sp)
        addi $sp, $sp, 1

        di                      ! Disable interrupts

        lw $k0, 0($sp)          ! Restore $k0
        addi $sp, $sp, 1

        reti

initsp: .fill 0xA000
ticks:  .fill 0xFFFF
range:  .fill 0xFFFE
maxval: .fill 0xFFFD
minval: .fill 0xFFFC
