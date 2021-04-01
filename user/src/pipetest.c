#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char STR[] = "hello pipe!";

int main() {
    // create pipe
    uint64 pipe_fd[2];
    int ret = pipe((void*)&pipe_fd);
    assert(ret == 0);
    printf("[parent] read end = %p, write end = %p\n", pipe_fd[0], pipe_fd[1]);
    if (fork() == 0) {
        // child process, read from parent
        // close write_end
        close(pipe_fd[1]);
        puts("[child] close write end");
        char buffer[32 + 1];
        int len_read = read(pipe_fd[0], buffer, 32);
        puts("[chile] read over");
        // assert(len_read < 32);
        buffer[len_read] = 0;
        assert(strncmp(buffer, STR, strlen(STR)) == 0);
        puts("Read OK, child process exited!");
        return 0;
    } else {
        // parent process, write to child
        // close read end
        close(pipe_fd[0]);
        printf("[parent] close read end\n");
        assert(write(pipe_fd[1], STR, strlen(STR)) == strlen(STR));
        printf("[parent] write over\n");
        // close write end
        close(pipe_fd[1]);
        int exit_code = 0;
        wait(-1, &exit_code);
        assert(exit_code == 0);
        puts("pipetest passed!");
    }
    return 0;
}
