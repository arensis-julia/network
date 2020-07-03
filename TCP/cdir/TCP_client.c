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
#define LEN 50

int main(int argc, char * argv[]) {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int c;
    SOCKET c;
    struct sockaddr_in serverAd;
    char * serverIp;
    char * fileName;
    unsigned short serverPort;
    char buffer[MAX] = {0, };
    int num = 0, fd;

    /* Check arguments */
    if(argc != 3) {
        fprintf(stderr, "[ERROR] Missing arguments: HOST_IP PORT_NUMBER\n");
        exit(-1);
    }
    serverIp = argv[1];
    serverPort = atoi(argv[2]);

    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    /* Create socket */
    if((c = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Cannot create socket\n");
        exit(-2);
    }

    /* Setting server socket */
    memset(&serverAd, 0, sizeof(serverAd));
    serverAd.sin_family = AF_INET;
    serverAd.sin_addr.s_addr = inet_addr(serverIp);
    serverAd.sin_port = htons(serverPort);

    /* Connecting */
    if((connect(c, (const struct sockaddr *)&serverAd, sizeof(serverAd))) < 0) {
        fprintf(stderr, "[ERROR] Connection failed\n");
        exit(-3);
    }

    /* Receive information about file (name, size) */
    // GET NAME
    if((num = recv(c, buffer, MAX, 0)) < 0) {
        fprintf(stderr, "[ERROR] Receive failed\n");
        exit(-4);
    }
    fileName = strdup(buffer);
    printf("Received file: %s\n", fileName);

    send(c, "ACK", MAX, 0);

    /* Create file to store the received data */
    if((fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0) {
        fprintf(stderr, "[ERROR] Open %s failed\n", fileName);
        exit(-5);
    }

    /* Receive file from server */
    memset(buffer, '\0', MAX);
    while((num = recv(c, buffer, MAX, 0)) > 0)
        write(fd, buffer, num);
    printf("SUCCESS!\n");

    /* Close */
    close(fd);
    close(c);
    // WINDOW ONLY
    WSACleanup();

    return 0;
}