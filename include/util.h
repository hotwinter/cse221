#define _GNU_SOURCE
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>

#define CPU_FREQ 2300000000.0
#define isb()        asm volatile("isb" : : : "memory")
/* All counters, including PMCCNTR_EL0, are disabled/enabled */

#define QUADD_ARMV8_PMCR_E      (1 << 0)
/* Reset all event counters, not including PMCCNTR_EL0, to 0 */
#define QUADD_ARMV8_PMCR_P      (1 << 1)
/* Reset PMCCNTR_EL0 to 0 */
#define QUADD_ARMV8_PMCR_C      (1 << 2)
/* Export of events is disabled/enabled */
#define QUADD_ARMV8_PMCR_X      (1 << 4)

#define    ARMV8_PMCR_MASK        0x3f     /* Mask for writable bits */

#define __NR_etimer 278
#define __NR_dtimer 279
#define __NR_escalate 280
#define KB(n) ((uint64_t)n*1024)
#define MB(n) ((uint64_t)n*1024*1024)
#define GB(n) ((uint64_t)n*1024*1024*1024)
#define PAGE_SIZE 4096

static inline unsigned int armv8_pmu_pmcr_read(void) {
    unsigned int val;

    asm volatile("mrs %0, pmcr_el0" : "=r" (val));
    return val;
}

static inline void armv8_pmu_pmcr_write(unsigned int val) {
    asm volatile("msr pmcr_el0, %0" : :"r" (val & ARMV8_PMCR_MASK));
}


static inline uint64_t armv8_read_CNTPCT_EL0(void) {
   uint64_t val;
   asm volatile("mrs %0, CNTVCT_EL0" : "=r" (val));

   return val;
}

static void reset_all_counters(void) {
    unsigned int val;

    val = armv8_pmu_pmcr_read();
    val |= QUADD_ARMV8_PMCR_P | QUADD_ARMV8_PMCR_C;
    armv8_pmu_pmcr_write(val);
}


static inline uint64_t read_counter(void) {
    uint64_t cval = 0;

    isb();
    asm volatile("mrs %0, PMCCNTR_EL0" : "+r"(cval));
    return cval;
}

static inline void read_ms(struct timeval *tim) {
    gettimeofday(tim, NULL);
}

static inline void enable_counters(void) {
    if (syscall(__NR_etimer) == -1) {
		printf("[-] enable counter in kernel failed\n");
		exit(EXIT_FAILURE);
	}
}

static inline void disable_counters(void) {
    if (syscall(__NR_dtimer) == -1) {
		printf("[-] disable counter in kernel failed\n");
		exit(EXIT_FAILURE);
	}
}

static inline void get_root(void) {
    if (syscall(__NR_escalate) == -1) {
		printf("[-] get_root failed\n");
		exit(EXIT_FAILURE);
	}
}

void set_cpu(int num) {
    cpu_set_t my_set;

    CPU_ZERO(&my_set);
    CPU_SET(num, &my_set);
    if (sched_setaffinity(0, sizeof(my_set), &my_set) != 0) {
        perror("[-] sched_setaffinity()");
        exit(EXIT_FAILURE);
    }
}

void mean() {
    int ret;

    ret = nice(-20);
    if (ret == -1) {
        perror("[-] setting priority");
        exit(EXIT_FAILURE);
    }
}

void setup(int cpu) {
    set_cpu(cpu);
    mean();
    enable_counters();
    reset_all_counters();
}

float cycle2ms(float cycle) {
    return cycle / (CPU_FREQ / 1000.0);
}

float average(float *measurement, int size) {
    float total = 0.0;
    int i;

    for(i = 0; i < size; i++) {
        total += measurement[i];
    }
    return total / size;
}

float standard_dev(float *measurement, int size, float average) {
    float sd = 0.0;
    int i;

    for(i = 0; i < size; i++) {
        sd += pow(measurement[i] - average, 2);
    }
    return sqrt(sd / size);
}

void clear_cache() {
    int ret;

    ret = system("echo 3 > /proc/sys/vm/drop_caches");
    if (ret < 0) {
        perror("system");
        exit(EXIT_FAILURE);
    }
}

float experiment(const char *desc, float (*func)(void*), void *args, int its, const char *units) {
    int i;
    float *res;
    float mean = 0;
    float sd = 0;

    printf("%s\n", desc);
    res = malloc(sizeof(float) * its);
    for(i = 0; i < its; i++) {
        reset_all_counters();
        res[i] = (*func)(args);
        //printf("Trial %d: %.2lf\n", i, res[i]);
    }

    mean = average(res, its);
    printf("Average: %.2lf%s\n", mean, units);
    sd = standard_dev(res, its, mean);
    printf("Standard Deviation: %.2lf%s\n", sd, units);
    free(res);
    return mean;
}
