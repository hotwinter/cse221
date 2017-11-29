#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define MEASUREMENT_OVERHEAD 104.77
// Loop overhead under -O2
#define LOOP_OVERHEAD 0.38
// 16MB
#define ALLOC_SIZE 16777216
#define CPU_FREQ 2300000000.0
// 2GB
#define FILE_SIZE 2147483648
// 16MB
#define PAGE_SEP 16777216

struct params {
    int stride_size;
    int array_size;
};

float access_time(void *args) {
    uint64_t prev, after;
    int i, indx;
    int *array;
    int array_size, stride_size;
    int it = 1000;
    
    array_size = ((struct params *) args)->array_size;
    stride_size = ((struct params *) args)->stride_size;
    array = (int *) malloc(sizeof(int) * array_size);
    for (i = 0; i < array_size; i++) {
        array[i] = (i + stride_size) % array_size;
    }
    indx = 0;
    prev = read_counter();
    for (i = 0; i < it; i++) {
        indx = array[indx];
    }
    after = read_counter();
    free(array);
    return (float) (after - prev - MEASUREMENT_OVERHEAD) / (float) it - LOOP_OVERHEAD;
}

void do_access_time() {
    struct params params;
    int array_size, i;
    int strides[8] = {4, 64, 128, 64*1024, 128*1024, 1024*1024, 4096*1024, 16*1024*1024};
    char desc[100];

    params.array_size = 4 * 1024;
    params.stride_size = 4;
    printf("1. RAM Access Time");
    for (i = 0; i < 8; i++) {
        params.stride_size = strides[i];
        for (array_size = 4 * 1024; array_size <= 128 * 1024 * 1024; array_size *= 2) {
            params.array_size = array_size;
            snprintf(desc, sizeof(desc), "Stride %d, Array Size %d", strides[i], array_size);
            experiment(desc, access_time, &params, 1, 0);
        }
        printf("\n");
    }
}

float read_time(void *args) {
    int *array;
    int sum, i;
    uint64_t prev, after;
    int array_size = ALLOC_SIZE / sizeof(int);
    
    array = (int *) malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        exit(1);
    }

    sum = 0;
    prev = read_counter();
    for(i = 0; i < array_size; i++) {
        sum += array[i];
    }
    after = read_counter();
    free(array);
    *(int *)args = sum;
    return (float) (after - prev - MEASUREMENT_OVERHEAD - array_size * LOOP_OVERHEAD);
}

float write_time(void *args) {
    int *array;
    int i;
    uint64_t prev, after;
    int array_size = ALLOC_SIZE / sizeof(int);
    
    array = (int *) malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        exit(1);
    }

    prev = read_counter();
    for(i = 0; i < array_size; i++) {
        array[i] = i;
    }
    after = read_counter();
    free(array);
    return (float) (after - prev - MEASUREMENT_OVERHEAD - array_size * LOOP_OVERHEAD) ;
}

float bandwidth(float cycles) {
    return (ALLOC_SIZE / 1024.0 / 1024.0 / 1024.0) * (CPU_FREQ / cycles);
}

void* page_fault_setup(char *fname, int *sfd) {
    int fd;
    void *res;

    fd = open(fname, O_RDWR);
    if(fd < 0) {
        perror("open");
        exit(1);
    }
    res = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(res == (void *) -1) {
        perror("mmap");
        exit(1);
    }
    *sfd = fd;
    return res;
}

float page_fault_time(void *addr) {
    int sum;
    int i;
    int64_t prev, after;
    int it = 100;
    int *newaddr;

    sum = 0;
    prev = read_counter();
    for(i = 0; i < it; i++) {
        newaddr = (int *)((char *)(addr) + (i + 1) * PAGE_SEP);
        sum += *newaddr;
    }
    after = read_counter();
    *(int *)addr = sum;
    return (float) (after - prev - MEASUREMENT_OVERHEAD) / (float) it - LOOP_OVERHEAD;
}

int main(int argc, char **argv) {
    int nu, fd;
    float readc, writec;
    float bw;
    void *addr;

    setup();
    /*do_access_time();
    readc = experiment("2.1 RAM Read Time", read_time, &nu, 100, 0);
    bw = bandwidth(readc);
    printf("Read Bandwidth is %.2f GB/s\n", bw);
    writec = experiment("2.2 RAM Write Time", write_time, NULL, 100, 0);
    bw = bandwidth(writec);
    printf("Write Bandwidth is %.2f GB/s\n", bw);
    */
    addr = page_fault_setup("hugefile", &fd);
    printf("file mapped at %p\n", addr);
    experiment("3 Page Fault Service Time", page_fault_time, addr, 10, 1);
    munmap(addr, FILE_SIZE);
    close(fd);
    disable_counters();
    return 0;
}
