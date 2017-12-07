#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define MEASUREMENT_OVERHEAD 104.77
// Loop overhead under -O2
#define LOOP_OVERHEAD 0.38
// 4MB
#define ARRAY_SIZE 8388608
//#define ALLOC_SIZE 128
//#define STEP_SIZE 2097152
#define STEP_SIZE 1
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
    for(i = 0; i < array_size; i++) {
        array[i] = (i + stride_size) % array_size;
    }
    indx = 0;
    prev = read_counter();
    for(i = 0; i < it; i++) {
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
    for(i = 0; i < 8; i++) {
        params.stride_size = strides[i];
        for(array_size = 4 * 1024; array_size <= 128 * 1024 * 1024; array_size *= 2) {
            params.array_size = array_size;
            snprintf(desc, sizeof(desc), "Stride %d, Array Size %d", strides[i], array_size);
            experiment(desc, access_time, &params, 1, 0, 0);
        }
        printf("\n");
    }
}

float read_time(void *args) {
    int *array;
    int *curr;
    int *last;
    int sum;
    uint64_t prev, after;
    int array_size = ARRAY_SIZE / sizeof(int);
    
    array = (int *) malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    last = array + array_size;
    curr = array;
    sum = 0;
    prev = read_counter();
    /*
    for(i = 0; i < array_size; i++) {
        sum += array[i];
    }*/
    while (curr != last) {
        sum += curr[0] + 
              curr[1] + 
              curr[2] + 
              curr[3] + 
              curr[4] + 
              curr[5] + 
              curr[6] + 
              curr[7] + 
              curr[8] + 
              curr[9] + 
              curr[10] + 
              curr[11] + 
              curr[12] + 
              curr[13] + 
              curr[14] + 
              curr[15] + 
              curr[16] + 
              curr[17] + 
              curr[18] + 
              curr[19] + 
              curr[20] + 
              curr[21] + 
              curr[22] + 
              curr[23] + 
              curr[24] + 
              curr[25] + 
              curr[26] + 
              curr[27] + 
              curr[28] + 
              curr[29] + 
              curr[30] + 
              curr[31];
        curr += 32;
    }
    after = read_counter();
    free(array);
    *(int *)args = sum;
    return (float) (after - prev - MEASUREMENT_OVERHEAD);
}

float write_time(void *args) {
    int *array;
    int *curr;
    int *last;
    uint64_t prev, after;
    int array_size = ARRAY_SIZE / sizeof(int);
    
    array = (int *) malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    last = array + array_size;
    curr = array;
    prev = read_counter();
    /*
    for(i = 0; i < array_size; i++) {
        array[i] = i;
    }*/
    while(curr != last) {
        curr[0] = 0;
        curr[1] = 1;
        curr[2] = 2;
        curr[3] = 3;
        curr[4] = 4;
        curr[5] = 5;
        curr[6] = 6;
        curr[7] = 7;
        curr[8] = 8;
        curr[9] = 9;
        curr[10] = 10;
        curr[11] = 11;
        curr[12] = 12;
        curr[13] = 13;
        curr[14] = 14;
        curr[15] = 15;
        curr[16] = 16;
        curr[17] = 17;
        curr[18] = 18;
        curr[19] = 19;
        curr[20] = 20;
        curr[21] = 21;
        curr[22] = 22;
        curr[23] = 23;
        curr[24] = 24;
        curr[25] = 25;
        curr[26] = 26;
        curr[27] = 27;
        curr[28] = 28;
        curr[29] = 29;
        curr[30] = 30;
        curr[31] = 31;
        curr += 32;
    }
    after = read_counter();
    free(array);
    return (float) (after - prev - MEASUREMENT_OVERHEAD) ;
}

float bandwidth(float cycles) {
    return (ARRAY_SIZE * (CPU_FREQ / cycles)) / 1024.0 / 1024.0 / 1024.0;
}

char* page_fault_setup(char *fname, int *sfd) {
    int fd, ret;
    void *res;

    fd = open(fname, O_RDWR);
    if(fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    res = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(res == (void *) -1) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    *sfd = fd;
    ret = system("echo 3 > /proc/sys/vm/drop_caches");
    if (ret != 0) {
        perror("ret");
        exit(EXIT_FAILURE);
    }
    return (char *) res;
}

float page_fault_time(void *args) {
    char c;
    int i, fd;
    uint64_t prev, after;
    int it = 100;
    char *addr;

    addr = page_fault_setup("hugefile", &fd);
    prev = read_counter();
    for(i = 0; i < it; i++) {
        c = addr[(i + 1) * PAGE_SEP];
    }
    after = read_counter();
    *addr = c;
    munmap(addr, FILE_SIZE);
    close(fd);
    return (float) (after - prev - MEASUREMENT_OVERHEAD) / (float) it - LOOP_OVERHEAD;
}

int main(int argc, char **argv) {
    int i;
    float readc, writec;
    float bw;

    setup(0);
    do_access_time();
    readc = experiment("2.1 RAM Read Time", read_time, &i, 100, 0, 0);
    bw = bandwidth(readc);
    printf("Read Bandwidth is %.2f GB/s\n", bw);
    writec = experiment("2.2 RAM Write Time", write_time, NULL, 100, 0, 0);
    bw = bandwidth(writec);
    printf("Write Bandwidth is %.2f GB/s\n", bw);
    get_root();
    experiment("3 Page Fault Service Time", page_fault_time, NULL, 10, 1, 1);
    disable_counters();
    return 0;
}
