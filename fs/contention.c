#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include "util.h"

#define FILE_SIZE KB(64)
#define BLOCK_SIZE PAGE_SIZE
#define MAX_CHILD 8

struct arguments {
    float (*f)(int, char*);
    int n;
};

float seq_read(int timers, char *fname) {
    unsigned int size = FILE_SIZE;
    char *buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
    int fd;
    uint64_t total = 0;
    ssize_t bytes;
	struct timeval prev, after;
    uint64_t total_time = 0;
 
    // O_DIRECT to avoid cache
    fd = open(fname, O_RDONLY | O_SYNC | O_DIRECT);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(total < size) {
        clear_cache();
		if (timers)	{
			read_ms(&prev);
        	bytes = read(fd, buf, BLOCK_SIZE);
			read_ms(&after);
            total_time += (after.tv_sec - prev.tv_sec) * 1000000.0 + (after.tv_usec - prev.tv_usec);
		} else {
        	bytes = read(fd, buf, BLOCK_SIZE);
		}
        if (bytes < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        total += bytes;
    }
    close(fd);
    free(buf);
	if (timers) {
		return total_time / (size / BLOCK_SIZE);
	}
	return 0;
}

float rand_read(int timers, char *fname) {
    unsigned int size = FILE_SIZE;
    char *buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
    int fd;
    uint64_t total = 0;
    unsigned int offset;
    ssize_t bytes;
    struct timeval prev, after;
    uint64_t total_time = 0;

    srand(time(NULL));
    // O_DIRECT to avoid cache
    fd = open(fname, O_RDONLY | O_SYNC | O_DIRECT);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(total < size) {
        offset = rand() % (FILE_SIZE / BLOCK_SIZE);
        lseek(fd, offset * BLOCK_SIZE, SEEK_SET);
        clear_cache();
		if (timers) {
			read_ms(&prev);
        	bytes = read(fd, buf, BLOCK_SIZE);
			read_ms(&after);
            total_time += (after.tv_sec - prev.tv_sec) * 1000000.0 + (after.tv_usec - prev.tv_usec);
		} else {
        	bytes = read(fd, buf, BLOCK_SIZE);
		}
        if (bytes < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        total += bytes;
    }
    close(fd);
    free(buf);
	if (timers) {
		return total_time / (size / BLOCK_SIZE);
	}
	return 0;
}

float parent(void *args) {
    int n = ((struct arguments *) args)->n;
	int i;
	pid_t pids[MAX_CHILD];
	float res;
	int status;
    char *files[8] = {"64k0", "64k1", "64k2", "64k3", "64k4", "64k5", "64k6", "64k7"};

	memset(pids, 0, sizeof(pid_t) * 8);
	// start processes
	for (i = 0; i < n; i++) {
	    if ((pids[i] = fork()) < 0) {
		    perror("fork");
		    exit(EXIT_FAILURE);
	    } else if (pids[i] == 0) {
		    ((struct arguments *) args)->f(0, files[i]);
		    exit(EXIT_SUCCESS);
	    }
	}

	res = ((struct arguments *) args)->f(1, "64k");

	// wait for children to exit
    i = 0;
	while(i < n) {
	    wait(&status);
        i++;
	}
	return res;
}

int main(int argc, char **argv) {
    int i;
    char desc[100];
	struct arguments args;

    setup(0);
    get_root();
	args.f = seq_read;
    printf("4.1 Sequential Readtime\n");
    for(i = 0; i < MAX_CHILD; i++) {
        snprintf(desc, sizeof(desc), "Child #%d:", i);
		args.n = i;
        experiment(desc, parent, &args, 10, "us");
    }
	args.f = rand_read;
    printf("2.2 Random Readtime\n");
    for(i = 0; i < MAX_CHILD; i++) {
        snprintf(desc, sizeof(desc), "Child #%d:", i);
		args.n = i;
        experiment(desc, parent, &args, 10, "us");
    }
    disable_counters();
    return 0;
}
