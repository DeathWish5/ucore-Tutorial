#include "defs.h"
#include "proc.h"
#include "riscv.h"
#include "trap.h"
#include "memory_layout.h"

static int app_cur, app_num;
static uint64 *app_info_ptr;
extern char _app_num[], _app_names[];
int fin = 0;

char names[20][100];

void batchinit() {
    char* s;
    app_info_ptr = (uint64 *) _app_num;
    app_cur = -1;
    app_num = *app_info_ptr;
    app_info_ptr++;

    s = _app_names;
    for(int i = 0; i < app_num; ++i) {
        int len = strlen(s);
        strncpy(names[i], (const char*)s, len);
        s += len + 1;
        trace("new name: %s\n", names[i]);
    }
}

int get_id_by_name(char* name) {
    for(int i = 0; i < app_num; ++i) {
        if(strncmp(name, names[i], 100) == 0)
            return i;
    }
    warn("not find such app\n");
    return -1;
}

uint64 alloc_ustack(struct proc *p) {
    p->ustack = 0;
    p->sz = USTACK_SIZE;
    if(mappages(p->pagetable, p->ustack, USTACK_SIZE,
                (uint64)kalloc(), PTE_R | PTE_W | PTE_U) < 0){;
        panic("");
    }
    return USTACK_SIZE;
}

void bin_loader(uint64 start, uint64 end, struct proc *p) {
    uint64 s = PGROUNDDOWN(start), e = PGROUNDUP(end);
    info("range : [%p, %p] start = %p\n", s, e, start);
    uint64 ustack_top = alloc_ustack(p);
    for(uint64 cur = s; cur < e; cur += PGSIZE) {
        void* page = kalloc();
        if(page == 0) {
            panic("");
        }
        memmove(page, (const void*)cur, PGSIZE);
        if (mappages(p->pagetable, p->sz, PGSIZE, (uint64)page, PTE_U | PTE_R | PTE_W | PTE_X) != 0) {
            panic("mappages\n");
        }
        p->sz += PGSIZE;
    }
    p->trapframe->epc = ustack_top + start - s;
    if((p->trapframe->epc & 3) != 0) {
        panic("invalid user program alignment");
    }
    p->trapframe->sp = ustack_top;
}

void loader(int id, void* p) {
    info("loader %s\n", names[id]);
    bin_loader(app_info_ptr[id], app_info_ptr[id+1], p);
}

int run_all_app() {
    struct proc *p = allocproc();
    p->parent = 0;
    int id = get_id_by_name("user_shell.bin");
    if(id < 0)
        panic("no user shell");
    loader(id, p);
    p->state = RUNNABLE;
    return 0;
}