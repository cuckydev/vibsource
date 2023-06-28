.set noreorder

.extern main

.extern __bss_start
.extern _end

.extern __CTOR_LIST__
.extern __CTOR_END__

.extern __DTOR_LIST__
.extern __DTOR_END__

.extern RunCtorList

.extern InitHeap

# Addresses
.section .sdata
.global addr_size
addr_size:
	.word 0x0001A000
.global addr_sp_size
addr_sp_size:
	.word 0x00000100
.global addr_load
addr_load:
	.word 0x80010000

# void _start(void);
.section .text._start
.global _start
.type _start, @function
_start:
	# Clear BSS
	la    $t0, __bss_start
	la    $t1, (_end - 4)
	blt   $t1, $t0, .Lclear_bss_done

.Lclear_bss_loop:
	sw    $zero, 0($t0)
	bne   $t0, $t1, .Lclear_bss_loop
	addiu $t0, 4
.Lclear_bss_done:

	# Set stack pointer to (*addr_size - 8) | 0x80000000
	lw    $sp, addr_size
	lui   $t0, 0x8000
	addiu $sp, -8
	or    $sp, $t0

	# Set heap to extend from _end to (addr_size - addr_sp_size)
	lw    $a1, addr_size
	lui   $t0, 0x8000
	or    $a1, $t0
	lw    $t0, addr_sp_size
	la    $a0, _end
	subu  $a1, $t0
	jal   InitHeap
	subu  $a1, $a0

	# Call main
	jal   main
	nop

	# Call destructors
	la    $a0, __DTOR_LIST__
	la    $a1, __DTOR_END__
	jal   RunCtorList
	nop

	# Exit
	break
