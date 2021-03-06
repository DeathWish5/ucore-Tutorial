#include "defs.h"

extern char ekernel[];
extern char sbss[];
extern char ebss[];

void clean_bss() {
    char* p;
    for(p = sbss; p < ebss; ++p)
        *p = 0;
}

void main() {
    clean_bss();
    printf("hello wrold!\n");
    trapinit();
    batchinit();
    run_next_app();
}