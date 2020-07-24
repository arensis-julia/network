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
#define localIP "127.0.0.1"

int main() {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int c;
    SOCKET c;
    struct sockaddr_in localServerAd;
    char * domainName;
    char buff[MAX] = {0, };
    int localServerLen;
    int fd;
    int num;

    /* Socket */
    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    if((c = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Socket failed\n");
        exit(-1);
    }

    /* Server address */
    memset(&localServerAd, 0, sizeof(localServerAd));
    localServerAd.sin_family = AF_INET;
    localServerAd.sin_addr.s_addr = inet_addr(localIP);
    localServerAd.sin_port = htons(localPort);
    localServerLen = sizeof(localServerAd);

    /* Open and read file */
    if((fd = open("address.txt", O_RDONLY)) < 0) {
        fprintf(stderr, "[ERROR] Opening address.txt failed\n");
        exit(-2);
    }
    memset(buff, '\0', MAX);
    if((num = read(fd, buff, sizeof(buff))) < 0) {
        fprintf(stderr, "[ERROR] Reading address.txt failed\n");
        exit(-3);
    }
    close(fd);
    domainName = strdup(buff);

    /* Send domain name to local server */
    sendto(c, domainName, strlen(domainName) + 1, 0, (struct sockaddr *)&localServerAd, localServerLen);
    printf("Sending domain name: %s\n", buff);

    /* Receive result */
    recvfrom(c, buff, MAX, 0, (struct sockaddr *)&localServerAd, &localServerLen);
    printf("Received: %s\n", buff);

    /* Store result */
    if((fd = open("result.txt", O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0) {
        fprintf(stderr, "[ERROR} Creating result.txt failed\n");
        exit(-4);
    }
    write(fd, buff, strlen(buff));
    printf("Saved result to the result.txt\n");

    close(fd);
    close(c);

    return 0;
}