#include "defs.h"
#include "trap.h"

static int app_cur, app_num;
static uint64* app_info_ptr;
extern char _app_num[], userret[], boot_stack[];

void batchinit() {
    app_info_ptr = (uint64*) _app_num;
    app_cur = -1;
    app_num = *app_info_ptr;
}

__attribute__ ((aligned (16))) char user_stack[4096*16];
__attribute__ ((aligned (16))) char trap_page[4096];

int run_next_app() {
    struct trapframe* trapframe = (struct trapframe*)trap_page;
    app_cur++;
    app_info_ptr++;
    if(app_cur >= app_num) {
        return -1;
    }
    printf("run app %d\n", app_cur);
    uint64 entry = *app_info_ptr;
    memset(trapframe, 0, 4096);
    trapframe->epc = entry;
    printf("entry = %p\n", trapframe->epc);
    trapframe->sp = (uint64) user_stack + 4096*16;
    usertrapret(trapframe, (uint64)boot_stack);
    return 0;
}