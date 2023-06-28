.set noreorder

.extern main

.extern __bss_start
.extern _end

.extern __CTOR_LIST__
.extern __CTOR_END__

.extern __DTOR_LIST__
.extern __DTOR_END__

.extern RunCtorList

.extern MemorySys_Init

# Addresses
.section .sdata
.global addr_size
addr_size:
	.word 0x00200000
.global addr_sp_size
addr_sp_size:
	.word 0x00004000
.global addr_heap_addr
addr_heap_addr:
	.word 0x00000000
.global addr_heap_size
addr_heap_size:
	.word 0x00000000

# State restore
.section .sdata
.global restore_sp
restore_sp:
	.word 0x00000000
.global restore_a0
restore_a0:
	.word 0x00000000
.global restore_a1
restore_a1:
	.word 0x00000000

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

	# Store some addresses
	sw    $ra, restore_sp
	sw    $a0, restore_a0
	sw    $a1, restore_a1

	# Set heap to extend from _end to (addr_size - addr_sp_size)
	lw    $a1, addr_size
	lui   $t0, 0x8000
	or    $a1, $t0
	lw    $t0, addr_sp_size
	la    $a0, _end
	sw    $a0, addr_heap_addr
	subu  $a1, $t0
	subu  $a1, $a0
	sw    $a1, addr_heap_size
	jal   MemorySys_Init
	nop

	# Call main
	lw    $a0, restore_a0
	lw    $a1, restore_a1
	nop
	jal   main
	nop

	# Call destructors
	la    $a0, __DTOR_LIST__
	la    $a1, __DTOR_END__
	jal   RunCtorList
	nop

	# Return
	lw    $ra, restore_sp
	nop
	jr    $ra
	nop
