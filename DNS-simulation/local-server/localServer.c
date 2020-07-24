#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
// LINUX:
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// WINDOW:
#include <winsock2.h>

#define MAX 1024
#define localPort 8000
#define globalPort 9000
#define globalIP "127.0.0.1"

int main() {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int lc, gc;
    SOCKET lc, gc;
    struct sockaddr_in localServerAd, localClientAd;
    struct sockaddr_in globalServerAd;
    char * domainName;
    char * pos;
    char * result;
    char buff[MAX] = {0. };
    int localServerLen, localClientLen;
    int globalServerLen;
    int fd;
    int num;

    // UDP

    printf("[CLIENT - LOCAL SERVER]\n");

    /* UDP Socket */
    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    if((lc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Cannot create UDP socket\n");
        exit(-1);
    }

    /* Server address (UDP) */
    memset(&localServerAd, 0, sizeof(localServerAd));
    localServerAd.sin_family = AF_INET;
    localServerAd.sin_port = htons(localPort);
    localServerAd.sin_addr.s_addr = htonl(INADDR_ANY);
    localServerLen = sizeof(localServerAd);
    localClientLen = sizeof(localClientAd);

    /* Bind */
    if((bind(lc, (struct sockaddr *)&localServerAd, localServerLen)) < 0) {
        fprintf(stderr, "[ERROR] Binding failed\n");
        exit(-2);
    }

    /* Receive domain name from client */
    memset(buff, '\0', MAX);
    recvfrom(lc, buff, MAX, 0, (struct sockaddr *)&localClientAd, &localClientLen);
    domainName = strdup(buff);
    printf("Domain name received: %s\n", domainName);

    /* Open and read file */
    if((fd = open("local_dns.txt", O_RDONLY)) < 0) {
        fprintf(stderr, "[ERROR] Opening local_dns.txt failed\n");
        exit(-3);
    }
    memset(buff, '\0', MAX);
    if((num = read(fd, buff, MAX)) < 0) {
        fprintf(stderr, "[ERROR] Reading local_dns.txt failed\n");
        exit(-4);
    }

    /* Finding domain name from file */
    if((pos = strstr(buff, domainName))) {
        printf("Found IP address!\n");
        strtok(pos, " \n");
        result = strtok(NULL, " \n");
    }
    else {
        printf("Cannot find IP address.\n");

        // TCP

        printf("[LOCAL SERVER - GLOBAL SERVER]\n");

        /* TCP Socket */
        if((gc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "[ERROR] Cannot create TCP socket\n");
            exit(-5);
        }

        /* Server address (TCP) */
        memset(&globalServerAd, 0, sizeof(globalServerAd));
        globalServerAd.sin_family = AF_INET;
        globalServerAd.sin_addr.s_addr = inet_addr(globalIP);
        globalServerAd.sin_port = htons(globalPort);
        globalServerLen = sizeof(globalServerAd);

        /* Connect */
        if(connect(gc, (const struct sockaddr *)&globalServerAd, globalServerLen) < 0) {
            fprintf(stderr, "[ERROR] Connecting failed\n");
            exit(-6);
        }

        /* Forward domain name to global server */
        send(gc, domainName, strlen(domainName) + 1, 0);
        printf("Forward domain name: %s\n", domainName);

        /* Receive result */
        memset(buff, '\0', MAX);
        recv(gc, buff, MAX, 0);
        result = strdup(buff);
        printf("Result: %s\n", result);

        close(gc);
    }

    // UDP

    printf("[CLIENT - LOCAL SERVER]\n");

    /* Send result to client */
    sendto(lc, result, strlen(result) + 1, 0, (struct sockaddr *)&localClientAd, localClientLen);
    printf("Sending result: %s\n", result);

    close(fd);
    close(lc);
    
    return 0;
}