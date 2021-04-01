#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    assert(wait(0, 0) < 0);
    printf("sys_wait without child process test passed!\n");
    printf("parent start, pid = %d!\n", getpid());
    int pid = fork();
    if (pid == 0) {
        // child process
        printf("hello child process!\n");
        return 100;
    } else {
        // parent process
        int xstate = 0;
        printf("ready waiting on parent process!\n");
        assert(pid == wait(0, &xstate));
        assert(xstate == 100);
        printf("child process pid = %d, exit code = %d\n", pid, xstate);
    }
    return 0;
}