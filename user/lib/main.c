#include <unistd.h>

extern int main(int, char**);

int __start_main(int argc, char** argv) {
    exit(main(argc, argv));
    return 0;
}
