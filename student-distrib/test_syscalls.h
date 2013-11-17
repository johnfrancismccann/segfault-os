#ifndef _TEST_SYSCALL_H
#define _TEST_SYSCALL_H

/* All calls return >= 0 on success or -1 on failure. */

/*  
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */ 
extern int32_t test_halt (uint8_t status);
extern int32_t test_execute (const uint8_t* command);
extern int32_t test_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t test_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t test_open (const uint8_t* filename);
extern int32_t test_close (int32_t fd);
extern int32_t test_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t test_vidmap (uint8_t** screen_start);
extern int32_t test_set_handler (int32_t signum, void* handler);
extern int32_t test_sigreturn (void);

enum signums {
	DIV_ZERO = 0,
	SEGFAULT,
	INTERRUPT,
	ALARM,
	USER1,
	NUM_SIGNALS
};

#endif
