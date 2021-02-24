#include "defs.h"

extern char sbss[];
extern char ebss[];

void clean_bss() {
    char* p;
    for(p = sbss; p < ebss; ++p)
        *p = 0;
}

void main() {
    clean_bss();
    trapinit();
    kinit();
    kvminit();
    batchinit();
    procinit();
    timerinit();
    run_all_app();
    printf("start scheduler!\n");
    scheduler();
}