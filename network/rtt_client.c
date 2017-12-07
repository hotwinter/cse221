#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include "util.h"

#define PACKET_SIZE 44
//#define DATA "0123456789abcdefghijklmnopqrstuvwxyzABCDEFG"

struct network_info {
    char *ip;
    int port;
};

int socket_setup(struct network_info *info) {
    struct sockaddr_in client; 
    int sock, ret;
    int one = 1;

    client.sin_addr.s_addr = inet_addr(info->ip);
    client.sin_family = AF_INET;
    client.sin_port = htons(info->port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("rtt sock create");
        exit(EXIT_FAILURE);
    }
    ret = connect(sock, (struct sockaddr *) &client, sizeof(client));
    if (ret == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(int));
    if (ret < 0) {
        perror("TCP_NODELAY");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void fill(char *buf, int size) {
    int fd;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("open random");
        exit(EXIT_FAILURE);
    }
    read(fd, buf, size);
    close(fd);
    //memcpy(buf, DATA, size);
}

float rtt(void *args) {
    int sock;
    char buf[PACKET_SIZE];
    struct timeval prev, after;
    float result;

    memset(buf, 0, sizeof(buf));
    fill(buf, sizeof(buf));
    sock = socket_setup((struct network_info *)args);
    reset_all_counters();
    read_ms(&prev);
    send(sock, buf, sizeof(buf), 0);
    recv(sock, buf, sizeof(buf), 0);
    read_ms(&after);
    close(sock);
    result = (after.tv_sec - prev.tv_sec) * 1000.0 + (after.tv_usec - prev.tv_usec) / 1000.0;
    return result;
}

int main(int argc, char **argv) {
    struct network_info info;

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    info.ip = argv[1];
    info.port = atoi(argv[2]);
    setup();
    experiment("1. RTT", rtt, &info, 10, 0, 1);
    disable_counters();
    return 0;
}
