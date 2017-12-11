#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "util.h"

#define FILE_SIZE GB(2)
#define BLOCK_SIZE PAGE_SIZE

struct arguments {
    unsigned int size;
    char *fname;
};

float seq_read(void *args) {
    unsigned int size = ((struct arguments *) args)->size;
    char *buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
    int fd;
    struct timeval prev, after;
    unsigned int total = 0;
    ssize_t bytes;
    uint64_t total_time = 0;
    char *fname = ((struct arguments *) args)->fname;

    // O_DIRECT to avoid cache
    fd = open(fname, O_RDONLY | O_SYNC | O_DIRECT);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(total < size) {
        clear_cache();
        read_ms(&prev);
        bytes = read(fd, buf, BLOCK_SIZE);
        read_ms(&after);
        if (bytes < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        total += bytes;
        total_time += (after.tv_sec - prev.tv_sec) * 1000000.0 + (after.tv_usec - prev.tv_usec);
    }
    close(fd);
    free(buf);
    return (float)(total_time) / (size / BLOCK_SIZE);
}

float rand_read(void *args) {
    unsigned int size = ((struct arguments *) args)->size;
    char *buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
    int fd;
    struct timeval prev, after;
    unsigned int total = 0;
    ssize_t bytes;
    uint64_t total_time = 0;
    unsigned int offset;
    char *fname = ((struct arguments *) args)->fname;

    srand(time(NULL));
    // O_DIRECT to avoid cache
    fd = open(fname, O_RDONLY | O_SYNC | O_DIRECT);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    while(total < size) {
        offset = rand() % (size / BLOCK_SIZE);
        lseek(fd, offset * BLOCK_SIZE, SEEK_SET);
        clear_cache();
        read_ms(&prev);
        bytes = read(fd, buf, BLOCK_SIZE); 
        read_ms(&after);
        if (bytes < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        total += bytes;
        total_time += (after.tv_sec - prev.tv_sec) * 1000000.0 + (after.tv_usec - prev.tv_usec);
    }
    close(fd);
    free(buf);
    return (float)(total_time) / (size / BLOCK_SIZE);
}

int main(int argc, char **argv) {
    int i;
    unsigned int sizes[] = {KB(16), KB(64), KB(128), KB(256), KB(512), MB(1), MB(2), MB(4), MB(8)};
    char desc[100];
    struct arguments args;

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    args.fname = argv[1];
    setup(0);
    get_root();
    printf("2.1 Sequential Readtime\n");
    for(i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        snprintf(desc, sizeof(desc), "%u: ", sizes[i]);
        args.size = sizes[i];
        experiment(desc, seq_read, &args, 10, "us");
    }
    printf("2.2 Random Readtime\n");
    for(i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        snprintf(desc, sizeof(desc), "%u: ", sizes[i]);
        args.size = sizes[i];
        experiment(desc, rand_read, &args, 10, "us");
    }
    disable_counters();
    return 0;
}
