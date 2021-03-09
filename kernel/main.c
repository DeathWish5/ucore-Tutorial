#include "defs.h"

extern char s_text[];
extern char e_text[];
extern char s_rodata[];
extern char e_rodata[];
extern char s_data[];
extern char e_data[];
extern char s_bss[];
extern char e_bss[];

void clean_bss() {
    char* p;
    for(p = s_bss; p < e_bss; ++p)
        *p = 0;
}

void main() {
    clean_bss();
    printf("\n");
    printf("hello wrold!\n");
    printf("stext: %p\n", s_text);
    printf("etext: %p\n", e_text);
    printf("sroda: %p\n", s_rodata);
    printf("eroda: %p\n", e_rodata);
    printf("sdata: %p\n", s_data);
    printf("edata: %p\n", e_data);
    printf("sbss : %p\n", s_bss);
    printf("ebss : %p\n", e_bss);
    printf("\n");
    shutdown();
}
