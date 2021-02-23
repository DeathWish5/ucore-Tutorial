#include "defs.h"

extern char stext[];
extern char etext[];
extern char srodata[];
extern char erodata[];
extern char sdata[];
extern char edata[];
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
    batchinit();
    procinit();
    run_all_app();
    printf("start scheduler!\n");
    scheduler();
}