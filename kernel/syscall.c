#include "defs.h"
#include "syscall_ids.h"
#include "trap.h"
#include "proc.h"

uint64 sys_write(int fd, uint64 va, uint len) {
    if (fd != 0)
        return -1;
    struct proc *p = curr_proc();
    char* str = (char*)useraddr(p->pagetable, va);
    int size = MIN(strlen(str), len);
    for(int i = 0; i < size; ++i) {
        console_putchar(str[i]);
    }
    return size;
}

uint64 sys_read(int fd, uint64 va, uint64 len) {
    if (fd != 0)
        return -1;
    struct proc *p = curr_proc();
    char* str = (char*)useraddr(p->pagetable, va);
    for(int i = 0; i < len; ++i) {
        int c = console_getchar();
        str[i] = c;
    }
    return len;
}

uint64 sys_exit(int code) {
    exit(code);
    return 0;
}

uint64 sys_sched_yield() {
    yield();
    return 0;
}

uint64 sys_getpid() {
    return curr_proc()->pid;
}

uint64 sys_clone() {
    return fork();
}

uint64 sys_exec(uint64 va) {
    struct proc* p = curr_proc();
    char* name = (char*)useraddr(p->pagetable, va);
    info("sys_exec %s\n", name);
    return exec(name);
}

uint64 sys_wait(int pid, uint64 va) {
    struct proc* p = curr_proc();
    int* code = (int*)useraddr(p->pagetable, va);
    return wait(pid, code);
}

uint64 sys_times() {
    return get_time_ms();
}

void syscall() {
    struct proc *p = curr_proc();
    struct trapframe *trapframe = p->trapframe;
    int id = trapframe->a7, ret;
    uint64 args[6] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
    trace("syscall %d args:%p %p %p %p %p %p\n", id, args[0], args[1], args[2], args[3], args[4], args[5]);
    switch (id) {
        case SYS_write:
            ret = sys_write(args[0], args[1], args[2]);
            break;
        case SYS_read:
            ret = sys_read(args[0], args[1], args[2]);
            break;
        case SYS_exit:
            ret = sys_exit(args[0]);
            break;
        case SYS_sched_yield:
            ret = sys_sched_yield();
            break;
        case SYS_getpid:
            ret = sys_getpid();
            break;
        case SYS_clone: // SYS_fork
            ret = sys_clone();
            break;
        case SYS_execve:
            ret = sys_exec(args[0]);
            break;
        case SYS_wait4:
            ret = sys_wait(args[0], args[1]);
            break;
        case SYS_times:
            ret = sys_times();
            break;
        default:
            ret = -1;
            warn("unknown syscall %d\n", id);
    }
    trapframe->a0 = ret;
    trace("syscall ret %d\n", ret);
}