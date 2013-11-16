.globl syscall_table

syscall_table:
	.long sys_halt
	.long sys_execute
	.long sys_read
	.long sys_write
	.long sys_open
	.long sys_close
	.long sys_getargs
	.long sys_vidmap
	.long sys_set_handler
	.long sys_sigreturn

