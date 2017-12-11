#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include "util.h"

struct network_info {
    char *ip;
    int port;
};

float msetup(void *args) {
    struct sockaddr_in client;
    int sock, ret;
    struct timeval prev, after;
    float result;
    struct network_info *info = (struct network_info*) args;

    client.sin_addr.s_addr = inet_addr(info->ip);
    client.sin_family = AF_INET;
    client.sin_port = htons(info->port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("rtt sock create");
        exit(EXIT_FAILURE);
    }

    read_ms(&prev);
    ret = connect(sock, (struct sockaddr *) &client, sizeof(client));
    read_ms(&after);
    if (ret == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    close(sock);
    result = (after.tv_sec - prev.tv_sec) * 1000.0 + (after.tv_usec - prev.tv_usec) / 1000.0;
    return result;
}

float mteardown(void *args) {
    struct sockaddr_in client;
    int sock, ret;
    struct timeval prev, after;
    float result;
    struct network_info *info = (struct network_info*) args;

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
    read_ms(&prev);
    close(sock);
    read_ms(&after);
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
    experiment("3.1 Setup overhead", msetup, &info, 10, "ms");
    experiment("3.2 Teardown overhead", mteardown, &info, 10, "ms");
    disable_counters();
    return 0;
}
