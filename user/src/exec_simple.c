#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    puts("execute hello.bin");
    exec("hello.bin");
    return 0;
}