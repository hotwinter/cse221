#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>

//#define PACKET_SIZE 104857600
#define PACKET_SIZE 0

int main(int argc, char **argv) {
    struct sockaddr_in server;
    int sock, csock, fd;
	int enable = 1;
    ssize_t size = 0;

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *buf = malloc(PACKET_SIZE);
    if (buf == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    memset(buf, 0, PACKET_SIZE);
    while(size < PACKET_SIZE) {
        size += read(fd, buf + size, PACKET_SIZE - size);
        printf("%ld\n", size);
    }
    printf("[+] done reading data\n");

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }
    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }
    if (listen(sock, 1) == -1) {
        perror("listen"); 
        return EXIT_FAILURE;
    }
    while(1) {
        csock = accept(sock, NULL, NULL);
        if (csock == -1) {
            perror("accept");
            return EXIT_FAILURE;
        }
        send(csock, buf, PACKET_SIZE, 0);
        close(csock);
    }
    close(sock);
    free(buf);
    return 0;
}
