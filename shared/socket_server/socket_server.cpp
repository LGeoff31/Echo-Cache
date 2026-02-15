#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

sockaddr_in buildServerAddress(int port) {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_add.sin_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    return server_addr;
}

int buildServerFileDescriptor(sockaddr_in server_addr) {
    int server_sockfd;

    // Create socket with IPv4 and TCP
    if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    // Bind socket to address
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind error");
        return 1;
    }

    // Start listening with a queue of 20
    if (listen(server_sockfd, 20) < 0) {
        perror("listen error");
        return 1;
    }

    return server_sockfd;
}

int buildSocketServer(int port) {
    sockaddr_in server_addr = buildServerAddress(port);
    int server_sockfd = buildServerFileDescriptor(server_addr);
    return server_sockfd;
}