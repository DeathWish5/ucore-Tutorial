#include "defs.h"
#include "syscall_ids.h"
#include "trap.h"

#define min(a, b) a < b ? a : b;

uint64 sys_write(int fd, char *str, uint len) {
    if (fd != 1)
        return -1;
    for(int i = 0; i < len; ++i) {
        console_putchar(str[i]);
    }
    return len;
}

uint64 sys_exit(int code) {
    printf("sysexit(%d)\n", code);
    run_next_app();
    printf("all apps over\n");
    shutdown();
    return 0;
}

extern char trap_page[];

void syscall() {
    struct trapframe *trapframe = (struct trapframe *) trap_page;
    int id = trapframe->a7, ret;
    printf("syscall %d\n", id);
    uint64 args[6] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
    switch (id) {
        case SYS_write:
            ret = sys_write(args[0], (char *) args[1], args[2]);
            break;
        case SYS_exit:
            ret = sys_exit(args[0]);
            break;
        default:
            ret = -1;
            printf("unknown syscall %d\n", id);
    }
    trapframe->a0 = ret;
    printf("syscall ret %d\n", ret);
}
