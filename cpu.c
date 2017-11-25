#include <stdint.h>
#include <stdio.h>
#include "util.h"


uint64_t readtime_overhead() {
    uint64_t prev, after;

    prev = read_counter();
    after = read_counter();
    return after - prev;
}

int main(int argc, char **argv) {

    setup();
    experiment(readtime_overhead, "1. Read Time Overhead");
    return 0;
}
