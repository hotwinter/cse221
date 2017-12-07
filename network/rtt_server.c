#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define PACKET_SIZE 44

int main(int argc, char **argv) {
    struct sockaddr_in server;
    int sock;
    int csock;
    char rbuf[PACKET_SIZE];
    char sbuf[PACKET_SIZE];
	int enable = 1;
    int test = 1;
    int count = 0;

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

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
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int)) < 0) {
        perror("TCP_NODELAY");
        exit(EXIT_FAILURE);
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
        memset(sbuf, 0, sizeof(sbuf));
        memset(rbuf, 0, sizeof(rbuf));
        csock = accept(sock, NULL, NULL);
        if (csock == -1) {
            perror("accept");
            return EXIT_FAILURE;
        }
        recv(csock, rbuf, sizeof(rbuf), 0);
        if (test) {
            printf("received\n");
            test = 0;
        }
        send(csock, sbuf, sizeof(sbuf), 0);
        close(csock);
        count += 1;
        //printf("%d\n", count);
    }
    close(sock);
    return 0;
}
