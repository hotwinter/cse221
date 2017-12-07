#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include "util.h"

// 1MB
#define PACKET_SIZE 1048576
// 100MB
#define TRANSFER_SIZE 104857600

struct network_info {
    char *ip;
    int port;
};

int socket_setup(struct network_info *info) {
    struct sockaddr_in client; 
    int sock, ret;

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
    return sock;
}

float bw(void *args) {
    int sock;
    char buf[PACKET_SIZE];
    struct timeval prev, after;
    float result;
    ssize_t size = 0;

    memset(buf, 0, PACKET_SIZE);
    sock = socket_setup((struct network_info *)args);
    read_ms(&prev);
    while(size < TRANSFER_SIZE) {
        size += recv(sock, buf, PACKET_SIZE, 0);
    }
    read_ms(&after);
    printf("%ld\n", size);
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
    setup(1);
    experiment("2. Bandwidth", bw, &info, 1, 0, 1);
    disable_counters();
    return 0;
}
