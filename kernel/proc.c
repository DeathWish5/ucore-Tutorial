#include "defs.h"
#include "proc.h"
#include "trap.h"
#include "riscv.h"

struct proc pool[NPROC];

__attribute__ ((aligned (16))) char kstack[NPROC][KSTACK_SIZE];

extern char boot_stack[];
struct proc* current_proc;
struct proc idle;


struct proc* curr_proc() {
    return current_proc;
}

void
procinit(void)
{
    struct proc *p;
    for(p = pool; p < &pool[NPROC]; p++) {
        p->state = UNUSED;
        p->kstack = (uint64) kstack[p - pool];
    }
    idle.kstack = (uint64)boot_stack;
    idle.pid = 0;
    idle.killed = 0;
}

int allocpid() {
    static int PID = 1;
    return PID++;
}

struct proc* allocproc(void)
{
    struct proc *p;
    for(p = pool; p < &pool[NPROC]; p++) {
        if(p->state == UNUSED) {
            goto found;
        }
    }
    return 0;

found:
    p->pid = allocpid();
    p->state = USED;
    memset(&p->context, 0, sizeof(p->context));
    memset((void*)p->kstack, 0, KSTACK_SIZE);
    p->context.ra = (uint64)usertrapret;
    p->context.sp = p->kstack + PGSIZE;
    return p;
}

void
scheduler(void)
{
    struct proc *p;

    for(;;){
        for(p = pool; p < &pool[NPROC]; p++) {
            if(p->state == RUNNABLE) {
                p->state = RUNNING;
                current_proc = p;
                printf("switch to next proc\n");
                swtch(&idle.context, &p->context);
            }
        }
    }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
    struct proc *p = curr_proc();
    if(p->state == RUNNING)
        panic("sched running");
    swtch(&p->context, &idle.context);
}

// Give up the CPU for one scheduling round.
void yield(void)
{
    current_proc->state = RUNNABLE;
    sched();
}

void exit(int code) {
    struct proc *p = curr_proc();
    printf("proc %d exit with %d\n", p->pid, code);
    p->state = UNUSED;
    finished();
    sched();
}