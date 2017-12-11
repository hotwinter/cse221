#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "util.h"

#define FILE_SIZE GB(2)
#define BLOCK_SIZE PAGE_SIZE

float file_cache(void *args) {
    uint64_t size = *((uint64_t *) args);
    char buf[BLOCK_SIZE];
    int fd;
    struct timeval prev, after;
    uint64_t total = 0;
    ssize_t bytes;
    uint64_t total_time = 0;

    // clear file cache
    clear_cache();

    fd = open("hugefile", O_RDONLY | O_SYNC);
    // read file into the file cache
    while(total < size) {
        bytes = read(fd, buf, BLOCK_SIZE); 
        total += bytes;
    }
    close(fd);

    fd = open("hugefile", O_RDONLY | O_SYNC);

    total = 0;
    while(total < size) {
        read_ms(&prev);
        bytes = read(fd, buf, BLOCK_SIZE); 
        read_ms(&after);
        total += bytes;
        total_time += (after.tv_sec - prev.tv_sec) * 1000000.0 + (after.tv_usec - prev.tv_usec);
    }
    close(fd);
    return (float)(total_time) / (size / BLOCK_SIZE);
}

int main(int argc, char **argv) {
    int i;
    uint64_t sizes[] = {KB(16), KB(64), KB(256), MB(1), MB(4), MB(16), MB(64), MB(256), MB(400), MB(600), MB(700), MB(750), MB(800), MB(850), MB(870), MB(880), MB(890), MB(900), MB(910), MB(920), MB(930), MB(950), GB(1), GB(1) + MB(10), GB(1) + MB(50), GB(1) + MB(100)};
    //uint64_t sizes[] = {MB(890), MB(900), MB(910), MB(920), MB(930), MB(950), GB(1)};
    char desc[100];

    setup(0);
    get_root();
    printf("1. File Cache\n");
    for(i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        snprintf(desc, sizeof(desc), "%" PRIu64 ": ", sizes[i]);
        experiment(desc, file_cache, sizes + i, 10, "us");
    }
    disable_counters();
    return 0;
}
