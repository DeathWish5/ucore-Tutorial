#include "defs.h"
#include "trap.h"
#include "proc.h"

static int app_cur, app_num;
static uint64* app_info_ptr;
extern char _app_num[];

int fin = 0;

int finished() {
    ++fin;
    if(fin >= app_num)
        panic("all apps over\n");
    return 0;
}

void batchinit() {
    app_info_ptr = (uint64*) _app_num;
    app_cur = -1;
    app_num = *app_info_ptr;
}

int run_all_app() {
    for(int i = 0; i < app_num; ++i) {
        app_cur++;
        app_info_ptr++;
        struct proc* p = allocproc();
        struct trapframe* trapframe = p->trapframe;
        printf("run app %d\n", app_cur);
        uint64 entry = *app_info_ptr;
        trapframe->epc = entry;
        trapframe->sp = (uint64) p->ustack + PAGE_SIZE;
        p->state = RUNNABLE;
    }
    return 0;
}