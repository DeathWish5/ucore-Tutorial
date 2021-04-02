#include <stdio.h>
#include <unistd.h>

int main() {
    exec("shell", 0);
    return 0;
}