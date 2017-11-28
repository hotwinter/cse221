#define _GNU_SOURCE
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <math.h>

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
#define SAMPLE 10
#define WARMUP 3


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

static inline void enable_counters(void) {
    if (syscall(__NR_etimer) == -1) {
		printf("[-] enable counter in kernel failed\n");
		exit(EXIT_FAILURE);
	}
}

static inline int disable_counters(void) {
    if (syscall(__NR_dtimer) == -1) {
		printf("[-] disable counter in kernel failed\n");
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

void setup() {
    set_cpu(0);
    mean();
    enable_counters();
    reset_all_counters();
}


void experiment(float (*func)(void), const char *desc, int its, int trial) {
    int i, j, indx;
    float val;
    float res[SAMPLE];
    float average = 0;
    float sd = 0;

    printf("%s\n", desc);
    // Cache warmup
    for(i = 0; i < SAMPLE + WARMUP; i++) {
        val = 0;
        reset_all_counters();
        for(j = 0; j < its; j++) {
            val += (*func)();
        }
        if (i >= WARMUP) {
            indx = i - WARMUP;
            res[indx] = ((float) val) / ((float) its);
            average += res[indx];
            if (trial) {
                printf("Trial %d: %.2lf\n", indx, res[indx]);
            }
        }
    }

    average /= ((float) SAMPLE);
    printf("Average: %.2lf\n", average);
    for(i = 0; i < SAMPLE; i++) {
        sd += pow(res[i] - average, 2);
    }
    sd = sqrt(sd / (float) SAMPLE);
    printf("Standard Deviation: %.2lf\n", sd);
}
