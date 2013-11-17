/* 
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */

#define SYS_HALT            1
#define SYS_EXECUTE         2
#define SYS_READ            3
#define SYS_WRITE           4
#define SYS_OPEN            5
#define SYS_CLOSE           6
#define SYS_GETARGS         7
#define SYS_VIDMAP          8
#define SYS_SET_HANDLER     9
#define SYS_SIGRETURN       10

#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL   %EBX          ;\
    MOVL    $number,%EAX  ;\
    MOVL    8(%ESP),%EBX  ;\
    MOVL    12(%ESP),%ECX ;\
    MOVL    16(%ESP),%EDX ;\
    INT $0x80         ;\
    POPL    %EBX          ;\
    RET

/* the system call library wrappers */
DO_CALL(test_halt,SYS_HALT)
DO_CALL(test_execute,SYS_EXECUTE)
DO_CALL(test_read,SYS_READ)
DO_CALL(test_write,SYS_WRITE)
DO_CALL(test_open,SYS_OPEN)
DO_CALL(test_close,SYS_CLOSE)
DO_CALL(test_getargs,SYS_GETARGS)
DO_CALL(test_vidmap,SYS_VIDMAP)
DO_CALL(test_set_handler,SYS_SET_HANDLER)
DO_CALL(test_sigreturn,SYS_SIGRETURN)
