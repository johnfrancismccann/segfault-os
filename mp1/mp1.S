
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %eax		# store callee-saved registers
		pushl %ecx
		pushl %edx
		call update_missiles
		call redraw_cities
		call redraw_xhairs
		popl %edx		# restore callee-saved registers
		popl %ecx
		popl %eax
		leave
		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		movl 8(%esp), %eax	# check cmd
		cmpl $0, %eax
		jb invalid		# invalid command
		cmpl $4, %eax
		ja invalid
		jmp *cmd+(,%eax,4)	# jump with jump table
	# ---jump table---
	cmd:
		.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame
		ret
	invalid:	
		movl $-1, %eax
		ret

# ----------------- Functions private to this file -------------------

update_missiles:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %ebx		# store callee-saved registers
		pushl %esi
		pushl %edi
		movl mp1_missile_list, %esi	# esi <- pointer to current missile
	loop:
		cmpl $0, %esi		# check if reached end of linked list (final node == 0)
		je end_loop

		# ---erase missile from screen
		pushl %eax		# store callee-saved registers
		pushl %ecx
		pushl %edx
		call calculate_offset	# returns offset in eax
		popl %edx
		popl %ecx
		add $4, %esp

		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		movb $32, %cl		# ASCII code for poke (space to erase)
		call mp1_poke
		popl %edx
		popl %ecx
		popl %eax

		# ---check if missile exploding or reached destination
		cmpl $0, EXPLODED(%esi)	# EXPLODED > 0 if exploding
		ja exploding				
	# update x and y if not exploding
		movl VX(%esi), %edx	# edx <- VX
		addl %edx, X(%esi)	# update x
		movl VY(%esi), %edx	# edx <- VY
		addl %edx, Y(%esi)	# update y
		movl DEST_X(%esi), %edx	# check if reached DEST_X
		movl X(%esi), %ecx
		shrl $16, %ecx
		cmpl %edx, %ecx		
		jne check_if_on_screen	# not at destination
		movl DEST_Y(%esi), %edx	# check if reached DEST_Y
		movl Y(%esi), %ecx
		shrl $16, %ecx
		cmpl %edx, %ecx		
		jne check_if_on_screen	# not at destination
		jmp exploding		# reached destination

	check_if_on_screen:
		movl X(%esi), %eax	# eax <- X
		shrl $16, %eax
		cmpl $0, %eax 
		jb free_missile		# X < 0	
		movl Y(%esi), %ebx	# ebx <- Y
		shrl $16, %ebx
		cmpl $0, %ebx 
		jb free_missile		# Y < 0	
		cmpl $79, %eax
		ja free_missile		# X > 79	
		cmpl $24, %ebx
		ja free_missile		# Y > 24	
	# redraw missiles
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		call calculate_offset	# returns offset in eax
		popl %edx
		popl %ecx
		add $4, %esp
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		movb C(%esi), %cl	# ASCII code for poke
		call mp1_poke
		popl %edx
		popl %ecx
		popl %eax
		jmp next_missile

		# ---call missile_explode if reached destination/exploding
	exploding:
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		pushl %esi		# push missile struct pointer for missile_explode
		call missile_explode
		popl %esi
		popl %edx
		popl %ecx
		addl $4, %esp
		cmpl $0, %eax		# check return value
		je no_status_change	# nonzero return value == status change
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		call mp1_notify_user
		popl %edx
		popl %ecx
		popl %eax
	no_status_change:
		decl EXPLODED(%esi)	# decrement EXPLODED field
		cmpl $0, EXPLODED(%esi)	# check if done exploding
		je free_missile		# done exploding if EXPLODED == 0
	# still exploding
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		call calculate_offset
		popl %edx
		popl %ecx
		addl $4, %esp
		movb $64, %cl		# exploded char (@) for poke
		pushl %ecx
		pushl %edx
		call mp1_poke
		popl %edx
		popl %ecx
		jmp next_missile

	free_missile:
		cmpl mp1_missile_list, %esi	# check if current missile is head
		je free_head
	# free non-head
		movl (%esi), %edx	# have previous missile point to next missile
		movl %edx, (%edi)
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		pushl %esi		# push exploded missile pointer
		call mp1_free		# free missile
		popl %esi
		popl %edx
		popl %ecx
		popl %eax
		movl (%edi), %esi	# have esi point to next missile
		# don't need to worry about moving edi because still points to previous missile
		jmp loop

	free_head:
		movl (%esi), %edx	# mp1_missile_list <- *NEXT (set new head)
		movl %edx, mp1_missile_list
		pushl %eax		# store caller-saved registers
		pushl %ecx
		pushl %edx
		pushl %esi		# push exploded missile pointer
		call mp1_free		# free missile
		popl %esi
		popl %edx
		popl %ecx
		popl %eax
		movl mp1_missile_list, %esi	# have esi point to new head
		jmp loop		# already arrived at next node so don't jump to next_missile 

	next_missile:
		cmpl %esi, mp1_missile_list	# check if current missile == head
		jne not_head		# if not head, already has edi assigned
	# current missile == head
		movl %esi, %edi		# edi <- current node (will be previous)
		jmp finish_loop
	not_head:
		movl (%edi), %edi	# edi <- next previous node
	finish_loop:
		movl (%esi), %esi	# esi <- next node
		jmp loop			

	end_loop:
		popl %edi			# restore registers
		popl %ebx
		popl %esi			
		leave
		ret

calculate_offset:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to new frame
		pushl %ebx		# store callee-saved registers
		pushl %esi
		pushl %edi
		movl X(%esi), %ecx	# get x location
		shrl $16, %ecx		# shift for video location
		movl Y(%esi), %edx	# get y location
		shrl $16, %edx		# shift for video location
		imull $160, %edx	# calculate offset for mp1_poke
		imull $2, %ecx
		addl %ecx, %edx		# edx <- offset
		movl %edx, %eax		# return offset in eax
		popl %edi			# restore registers
		popl %esi
		popl %ebx			
		leave
		ret

redraw_cities:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %ebx		# store callee-saved registers
		pushl %esi
		pushl %edi
		movl base_alive, %ebx	# ebx <- base_alive array
	# check left base
		cmpb $0, %bl		# check 1st byte (rightmost/least significant byte)
		je left_base_dead
	# left base alive
		leal base_pic, %esi	# esi <- *(base_pic string)
		pushl $18		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack
		jmp check_middle_base
	left_base_dead:
		leal dead_base_pic, %esi	# esi <- *(dead_base_pic string)
		pushl $18		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack
	
	check_middle_base:
		movl base_alive, %ebx	# ebx <- base_alive array
		shrl $8, %ebx # check 2nd byte
		cmpb $0, %bl		
		je middle_base_dead
	# middle base alive
		leal base_pic, %esi	# esi <- *(base_pic string)
		pushl $38		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack
		jmp check_right_base
	middle_base_dead:
		leal dead_base_pic, %esi	# esi <- *(dead_base_pic string)
		pushl $38		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack

	check_right_base:
		movl base_alive, %ebx	# ebx <- base_alive array
		shrl $12, %ebx		# check 3rd byte	
		cmpb $0, %bl		
		je right_base_dead
	# right base alive
		leal base_pic, %esi	# esi <- *(base_pic string)
		pushl $58		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack
		jmp done_checking_bases
	right_base_dead:
		leal dead_base_pic, %esi	# esi <- *(dead_base_pic string)
		pushl $58		# push leftmost x of base
		call draw_base
		addl $4, %esp		# pop x off stack

	done_checking_bases:
		popl %edi		# restore callee-saved registers
		popl %esi
		popl %ebx
		leave
		ret

# pointer to pic string in esi
# draw_base(x) [leftmost x]
draw_base:
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx		# store callee-saved registers
		pushl %esi
		pushl %edi
		movl 8(%ebp), %edx	# edx <- x coordinate
		xorl %ebx, %ebx		# ebx <- counter (init. 0)
	base_loop:
		cmpl $5, %ebx		# check if drew all base chars
		jae end_base_loop
		imull $2, %edx, %eax	# eax <- offset
		addl $3840, %eax	# add 3840 since row 24
		movb (%esi,%ebx,1), %cl # cl <- base pic char
		pushl %eax		# save registers
		pushl %ecx
		pushl %edx
		call mp1_poke
		popl %edx		# restore registers
		popl %ecx
		popl %eax
		incl %ebx		# increment loop counter
		incl %edx		# increment x-coordinate
		jmp base_loop
	end_base_loop:
		popl %edi		# restore callee-saved registers
		popl %esi
		popl %ebx
		leave
		ret

redraw_xhairs:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %esi		# callee-saved registers
		pushl %ebx
		pushl %edi
		movl crosshairs_x, %ecx	# get x location
		movl crosshairs_y, %edx	# get y location
		imull $160, %edx	# calculate offset for mp1_poke
		imull $2, %ecx
		addl %ecx, %edx		# edx <- offset
		movl %edx, %eax
		pushl %eax		# save registers
		pushl %ecx
		pushl %edx
		movb $43, %cl
		call mp1_poke
		popl %edx
		popl %ecx
		popl %eax
		popl %edi			# restore registers
		popl %ebx
		popl %esi			
		leave
		ret

mp1_ioctl_startgame:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %esi		# callee-saved registers
		pushl %ebx
		pushl %edi
		movl $40, crosshairs_x 		# crosshairs set to
		movl $12, crosshairs_y		# middle of screen
		movl $0x00010101, base_alive	# set bases to alive (x1110)
		movl $0x70000, mp1_score	# init. to all bases alive, score 0
		popl %edi			# restore registers
		popl %ebx
		popl %esi			
		leave
		ret

mp1_ioctl_addmissile:
		pushl %ebp			# save old frame pointer
		movl %esp, %ebp			# point to a new frame
		pushl %esi			# callee-saved registers
		pushl %ebx
		pushl %edi
		movl mp1_missile_list, %esi	# save old missile head in esi
		pushl %eax			# caller-save registers
		pushl %ecx
		pushl %edx
		pushl $33			# push 33 (bytes) on stack
		call mp1_malloc			# for new missile
		popl %edx
		popl %ecx
		addl $4, %esp
		cmpl $0, %eax			# check return value
		je malloc_fail			# failed malloc if return is 0	
		movl %eax, %ebx			# save pointer to new missile in ebx
		pushl %eax			# caller-saved regs
		pushl %ecx
		pushl %edx
		pushl $33			# push 'n'
		pushl 8(%ebp)			# push user_missile pointer as 'from'
		pushl %eax			# push return pointer from malloc as 'to'
		call mp1_copy_from_user
		popl %edx
		popl %ecx
		addl $4, %esp			# pop stack parameters
		cmpl $0, %eax			# check return value for error
		jne add_fail			# error if return value != 0
		movl %ebx, mp1_missile_list	# have list point to new missile
		movl %esi, (%ebx)		# set 'next' of new missile
		movl $0, %eax			# return 0 if success
		popl %edi			# restore registers
		popl %ebx
		popl %esi			
		movl mp1_missile_list,  %esi	# have esi point to head (new missile)
		leave
		ret

	add_fail:
		pushl %eax			# caller-saved regs
		pushl %ecx
		pushl %edx
		pushl %ebx		# push malloc'ed pointer
		call mp1_free
		popl %ebx
		popl %edx
		popl %ecx
		popl %eax
		movl $-1, %eax		# -1 return value if fail
		popl %edi		# restore registers
		popl %ebx
		popl %esi		
		leave
		ret

	malloc_fail:
		movl $-1, %eax		# -1 return value if fail
		popl %edi		# restore registers
		popl %ebx
		popl %esi		
		leave
		ret

mp1_ioctl_movexhairs:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %esi		# callee-saved registers
		pushl %ebx
		pushl %edi
	# erase previous crosshair
		movl crosshairs_x, %ecx	# get x location
		movl crosshairs_y, %edx	# get y location
		imull $160, %edx	# calculate offset for mp1_poke
		imull $2, %ecx
		addl %ecx, %edx		# edx <- offset
		pushl %eax			# caller-saved regs
		pushl %ecx
		pushl %edx
		movl %edx, %eax		# return offset in eax
		movb $32, %cl		# ASCII code for poke (space to erase)
		call mp1_poke
		popl %edx
		popl %ecx
		popl %eax
	# draw new crosshair
		movl 8(%ebp), %ecx	# get xhair_delta		
		shll $16, %ecx
		sarl $16, %ecx		# ecx <- change in x (arithmetic shift)
		movl 8(%ebp), %edx	# get xhair_delta		
		sarl $16, %edx		# edx <- change in y (arithmetic shift)
		addl %ecx, crosshairs_x
	# prevent crosshair from moving horizontally off screen
		cmpl $0, crosshairs_x	# if < 0
		jl undo_x_change
		cmpl $79, crosshairs_x	# if > 79
		jg undo_x_change
		jmp move_y_xhairs
	undo_x_change:
		subl %ecx, crosshairs_x
	move_y_xhairs:
		addl %edx, crosshairs_y
	# prevent crosshair from moving vertically off screen
		cmpl $0, crosshairs_y	# if < 0
		jl undo_y_change
		cmpl $24, crosshairs_y	# if > 24
		jg undo_y_change
		jmp redraw_crosshairs
	undo_y_change:
		subl %edx, crosshairs_y
	redraw_crosshairs:
		pushl %eax		# caller-saved regs
		pushl %ecx
		pushl %edx
		call redraw_xhairs	# redraw with new values
		popl %edx
		popl %ecx
		popl %eax
		movl $0, %eax		# return 0
		popl %edi		# restore registers
		popl %ebx
		popl %esi		
		leave
		ret

mp1_ioctl_getstatus:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %esi		# callee-saved registers
		pushl %ebx
		pushl %edi
		movl mp1_score, %edx	# edx <- ptr to score (low 16 bits)
		movl base_alive, %ecx	# ecx <- base_alive
	# check left
		cmpb $0, %cl		# check if left base alive
		je left_dead
		jmp check_mid
	left_dead:
		andl $0x6ffff, %edx	# set left base bit to 0
	check_mid:
		shrl $8, %ecx		# to check byte 1 (mid base)
		cmpb $0, %cl
		je mid_dead
		jmp check_right

	mid_dead: 
		andl $0x5ffff, %edx	# set mid base bit to 0	
	check_right:
		shrl $8, %ecx		# to check byte 2 (right base)
		cmpb $0, %cl
		je right_dead
		jmp skip

	right_dead:
		andl $0x3ffff, %edx	# set right base bit to 0	
	skip:
		movl %edx, mp1_score
		leal mp1_score, %esi	# esi <- ptr to base_alive/score
		pushl %edx
		pushl %ecx
		pushl $4		# push 'n'
		pushl %esi		# push 'from' (ptr to base_alive/score)
		pushl 8(%ebp)		# push 'to' (user_status)
		call mp1_copy_to_user
		movl %eax, %ebx		# store return value in ebx
		addl $12, %esp		# pop parameters
		popl %ecx
		popl %edx
		cmpl $0, %ebx		# check if error (nonzero)
		jne copy_error
		movl $0, %eax		# return 0 if no error
		jmp no_error
	copy_error:
		movl $-1, %eax		# return -1 if error
	no_error:
		popl %edi		# restore registers
		popl %ebx
		popl %esi		
		leave
		ret

mp1_ioctl_endgame:
		pushl %ebp		# save old frame pointer
		movl %esp, %ebp		# point to a new frame
		pushl %esi		# callee-saved registers
		pushl %ebx
		pushl %edi
		movl mp1_missile_list, %esi	# esi <- ptr to list head
	loop_list:
		cmpl $0, %esi		# check if empty list (points to 0)
		je end_loop_list
		movl (%esi), %esi	# move ptr to next node
		pushl mp1_missile_list	# push for freeing
		call mp1_free
		movl %esi, mp1_missile_list

	end_loop_list:
		popl %edi		# restore registers
		popl %ebx
		popl %esi		
		movl $0, %eax		# return 0 for success
		leave
		ret
