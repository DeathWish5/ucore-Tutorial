#include "defs.h"
#include "syscall_ids.h"
#include "trap.h"
#include "proc.h"

#define min(a, b) a < b ? a : b;

uint64 console_write(uint64 va, uint64 len) {
    struct proc *p = curr_proc();
    char str[200];
    int size = copyinstr(p->pagetable, str, va, MIN(len, 200));
    for(int i = 0; i < size; ++i) {
        console_putchar(str[i]);
    }
    return size;
}

uint64 console_read(uint64 va, uint64 len) {
    struct proc *p = curr_proc();
    char str[200];
    for(int i = 0; i < len; ++i) {
        int c = console_getchar();
        str[i] = c;
    }
    copyout(p->pagetable, va, str, len);
    return len;
}

uint64 sys_write(int fd, uint64 va, uint64 len) {
    if(fd == 0) {
        return console_write(va, len);
    }
    struct proc *p = curr_proc();
    struct file *f = p->files[fd];
    if(f->type == FD_PIPE) {
        info("write to pipe at %p\n", f->pipe);
        return pipewrite(f->pipe, va, len);
    }
    error("unknown file type %d\n", f->type);
    panic("syswrite: unknown file type\n");
    return -1;
}

uint64 sys_read(int fd, uint64 va, uint64 len) {
    if(fd == 0) {
        return console_read(va, len);
    }
    struct proc *p = curr_proc();
    struct file *f = p->files[fd];
    if(f->type == FD_PIPE) {
        info("read to pipe at %p\n", f->pipe);
        return piperead(f->pipe, va, len);
    }
    error("unknown file type %d\n", f->type);
    panic("sysread: unknown file type\n");
    return -1;
}

uint64
sys_pipe(uint64 fdarray) {
    info("init pipe\n");
    struct proc *p = curr_proc();
    uint64 fd0, fd1;
    struct file* f0, *f1;
    if(f0 < 0 || f1 < 0) {
        return -1;
    }
    f0 = filealloc();
    f1 = filealloc();
    if(pipealloc(f0, f1) < 0)
        return -1;
    fd0 = fdalloc(f0);
    fd1 = fdalloc(f1);
    if(copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
       copyout(p->pagetable, fdarray+sizeof(uint64), (char *)&fd1, sizeof(fd1)) < 0){
        fileclose(f0);
        fileclose(f1);
        p->files[fd0] = 0;
        p->files[fd1] = 0;
        return -1;
    }
    return 0;
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
    info("fork!\n");
    return fork();
}

uint64 sys_exec(uint64 va) {
    struct proc* p = curr_proc();
    char name[200];
    copyinstr(p->pagetable, name, va, 200);
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

uint64 sys_close(int fd) {
    if(fd == 0)
        return 0;
    struct proc *p = curr_proc();
    struct file *f = p->files[fd];
    if(f->type != FD_PIPE) {
        error("unknown file type %d\n", f->type);
        panic("fileclose: unknown file type\n");
    }
    fileclose(f);
    p->files[fd] = 0;
    return 0;
}

void syscall() {
    struct proc *p = curr_proc();
    struct trapframe *trapframe = p->trapframe;
    int id = trapframe->a7, ret;
    uint64 args[7] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5, trapframe->a6};
    trace("syscall %d args:%p %p %p %p %p %p %p\n", id, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
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
        case SYS_pipe2:
            ret = sys_pipe(args[0]);
            break;
        case SYS_close:
            ret = sys_close(args[0]);
            break;
        default:
            ret = -1;
            warn("unknown syscall %d\n", id);
    }
    trapframe->a0 = ret;
    trace("syscall ret %d\n", ret);
}
