#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int NUM = 30;

int main() {
    for (int i = 0; i < NUM; ++i) {
        int pid = fork();
        if (pid == 0) {
            int current_time = get_time();
            unsigned long long sleep_length =
                current_time * current_time % 1000 + 1000;
            printf("pid %d sleep for %p ms\n", getpid(), sleep_length);
            sleep(sleep_length);
            printf("pid %d OK!\n", getpid());
            exit(0);
        }
    }

    int xstate = 0;
    for (int i = 0; i < NUM; ++i) {
        assert(wait(-1, &xstate) > 0);
        assert(xstate == 0);
    }
    assert(wait(-1, &xstate) < 0);
    printf("forktest2 test passed!\n");
    return 0;
}
