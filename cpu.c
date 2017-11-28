#include <stdint.h>
#include <stdio.h>
#include "util.h"
#include <pthread.h>
#include <sys/wait.h>
#include <assert.h>

#define MEASUREMENT_OVERHEAD 104.77

struct params {
    int rpipe;
    uint64_t result;
};

float readtime_overhead() {
    uint64_t prev, after;

    prev = read_counter();
    after = read_counter();
    return (float)(after - prev);
}

float loop_overhead() {
    int i;
    uint64_t prev, after;
    prev = read_counter();
    for(i = 0; i < 10000; i++) {
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD)) / 10000.0;
}

void func0() {}

void func1(int a1) {}

void func2(int a1, int a2) {}

void func3(int a1, int a2, int a3) {}

void func4(int a1, int a2, int a3, int a4) {}

void func5(int a1, int a2, int a3, int a4, int a5) {}

void func6(int a1, int a2, int a3, int a4, int a5, int a6) {}

void func7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {}

float func0_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func0();
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func1_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func1(1);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func2_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func2(1, 2);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func3_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func3(1, 2, 3);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func4_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func4(1, 2, 3, 4);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func5_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func5(1, 2, 3, 4, 5);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func6_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func6(1, 2, 3, 4, 5, 6);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

float func7_overhead() {
    uint64_t prev, after;
    int i;

    prev = read_counter();
    for(i = 0; i < 10000; i++) {
        func7(1, 2, 3, 4, 5, 6, 7);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD) / 10000.0);
}

void do_procedure_experiment() {
    int i;
    char desc[100];
    float (*funcs[8])(void) = {func0_overhead, func1_overhead, func2_overhead, func3_overhead,
                               func4_overhead, func5_overhead, func6_overhead, func7_overhead};

    memset(desc, 0, 100);
    for(i = 0; i < 8; i++) {
        snprintf(desc, sizeof(desc), "2. Procedure call overhead (%d)", i);
        experiment(funcs[i], desc, 10000, 0);
    }
}

float process_overhead() {
    uint64_t prev, after;
    int status;
    pid_t pid;

    prev = read_counter();
    pid = fork();
    // Child
    if(pid == 0) {
        _exit(0);
    } else {
        waitpid(pid, &status, 0);
    }
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD));
}

void *child_thread(void *arg) {
    pthread_exit(NULL);
}

float kernel_thread_overhead() {
    uint64_t prev, after;
    pthread_t thread;

    prev = read_counter();
    pthread_create(&thread, NULL, child_thread, NULL);
    pthread_join(thread, NULL);
    after = read_counter();
    return ((float) (after - prev - MEASUREMENT_OVERHEAD));
}

float context_switch_user() {
    int p[2];
    int shared[2];
    pid_t pid;
    uint64_t prev, after;
    char buf[20];
    int status, rpipe, wpipe;
    const char *message = "test";
    int mlen = strlen(message);

    memset(buf, 0, sizeof(buf));
    if (pipe(p) == -1) {
        perror("pipe");
        exit(1);
    }
    if (pipe(shared) == -1) {
        perror("pipe");
        exit(1);
    }
    rpipe = p[0];
    wpipe = p[1];
    pid = fork();
    if (pid == 0) {
        close(shared[0]);
        read(rpipe, buf, mlen);
        after = read_counter();
        snprintf(buf, sizeof(buf), "%llu", (long long unsigned int) after);
        write(shared[1], buf, sizeof(buf));
        close(rpipe);
        close(wpipe);
        exit(0);
    } else {
        prev = read_counter();
        write(wpipe, message, mlen);
        // forcing context switch
        sleep(0.01);
        read(shared[0], buf, sizeof(buf));
        after = atoll(buf);
        waitpid(pid, &status, 0);
    }
    close(rpipe);
    close(wpipe);
    close(shared[0]);
    close(shared[1]);
    return (after - prev - MEASUREMENT_OVERHEAD);
}

void *context_switch_child(void *arg) {
    char buf[20];
    const char *message = "test";
    int mlen = strlen(message);
    uint64_t after;
    struct params *params;

    memset(buf, 0, sizeof(buf));
    params = (struct params *) arg;
    read(params->rpipe, buf, mlen);
    after = read_counter();
    params->result = after;
    assert(strcmp(buf, message) == 0);
    pthread_exit(NULL);
}

void *kernel_thread(void *arg) {
    int p[2];
    pthread_t thread; 
    uint64_t prev, after;
    char buf[20];
    int rpipe, wpipe;
    const char *message = "test";
    int mlen = strlen(message);
    struct params params;

    memset(buf, 0, sizeof(buf));
    if (pipe(p) == -1) {
        perror("pipe");
        exit(1);
    }
    rpipe = p[0];
    wpipe = p[1];
    params.rpipe = rpipe;
    params.result = 0;
    pthread_create(&thread, NULL, context_switch_child, &params);
    prev = read_counter();
    write(wpipe, message, mlen);
    //forcing context switch
    sleep(0.01);
    pthread_join(thread, NULL);
    after = params.result;
    close(rpipe);
    close(wpipe);
    return (void *) (after - prev);
}

float context_switch_kernel() {
    pthread_t thread;
    uint64_t res;

    pthread_create(&thread, NULL, kernel_thread, NULL);
    pthread_join(thread, (void *) &res);
    return res - MEASUREMENT_OVERHEAD;
}

int main(int argc, char **argv) {

    setup();
    experiment(readtime_overhead, "1.1 Read Time Overhead", 10000, 1);
    experiment(loop_overhead, "1.2 Loop Overhead", 10000, 1);
    do_procedure_experiment();
    experiment(process_overhead, "3.1 Process Overhead", 10000, 1);
    experiment(kernel_thread_overhead, "3.2 Kernel Process Overhead", 10000, 1);
    experiment(context_switch_user, "4.1 Process Context Switch Overhead", 1000, 1);
    experiment(context_switch_kernel, "4.2 Kernel Thread Context Switch Overhead", 1000, 1);
    return 0;
}
