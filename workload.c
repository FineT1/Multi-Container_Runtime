// workload_mem.c
#include <stdlib.h>
#include <unistd.h>

int main() {
    while (1) {
        malloc(1024 * 1024); // allocate 1MB repeatedly
        usleep(10000);
    }
}
