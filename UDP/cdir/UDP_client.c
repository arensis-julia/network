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

int main(int argc, char * argv[]) {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int c;
    SOCKET c;
    struct sockaddr_in serverAd;
    char * serverIP;
    char * fileName;
    unsigned short serverPort;
    char buffer[MAX] = {0, };
    int serverLen;
    int fd, num, i = 0;
    int isNum = 0;

    /* Check arguments */
    if(argc != 3) {
        fprintf(stderr, "[ERROR] Missing arguments: HOST_IP PORT_NUMBER\n");
        exit(-1);
    }
    serverIP = argv[1];
    serverPort = atoi(argv[2]);

    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    /* Create socket */
    if((c = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Cannot create socket\n");
        exit(-2);
    }

    /* Setting server socket */
    memset(&serverAd, 0, sizeof(serverAd));
    serverAd.sin_family = AF_INET;
    serverAd.sin_addr.s_addr = inet_addr(serverIP);
    serverAd.sin_port = htons(serverPort);
    serverLen = sizeof(serverAd);

    /* Send Request FLIST */
    sendto(c, "FLIST", 6, 0, (struct sockaddr *)&serverAd, serverLen);
    printf("Send request for FILE LIST\n");

    /* Receive file list */
    printf("\n====================FILE LIST====================\n");
    memset(buffer, '\0', MAX);
    while(strcmp(buffer, "END")) {
        memset(buffer, '\0', MAX);
        if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&serverAd, &serverLen)) < 0) {
            fprintf(stderr, "[ERROR] Receiving file list failed\n");
            exit(-3);
        }
        if(strcmp(buffer, "END")) {
            printf("[%d] %s\n", i, buffer);
            i++;
        }
    }
    printf("==================================================\n\n");

    /* Send file request */
    memset(buffer, '\0', MAX);
    printf("Request: ");
    fflush(stdout);
    num = read(0, buffer, MAX);
    buffer[num - 1] = '\0';
    fileName = strdup(buffer);
    sendto(c, fileName, strlen(fileName) + 1, 0, (struct sockaddr *)&serverAd, serverLen);

    for(i=0; i<strlen(fileName); i++) {
        if(fileName[i] < '0' || fileName[i] > '9')
            isNum = 1;
    }

    /* Receive response */
    memset(buffer, '\0', MAX);
    if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&serverAd, &serverLen)) < 0) {
        fprintf(stderr, "[ERROR] Receiving sign failed\n");
        exit(-4);
    }
    if(!strcmp(buffer, "INVALID")) {
        fprintf(stderr, "Invalid Request\n");
        exit(-5);
    }
    if(isNum == 0) {
        memset(buffer, '\0', MAX);
        if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&serverAd, &serverLen)) < 0) {
            fprintf(stderr, "[ERROR] Receiving fileName failed\n");
            exit(-6);
        }
        fileName = strdup(buffer);
        printf("Requested File: %s\n", fileName);
    }

    /* Receive file */
    if((fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0) {
        fprintf(stderr, "[ERROR] Open %s failed\n", fileName);
        exit(-7);
    }
    memset(buffer, '\0', MAX);
    while(strcmp(buffer, "END")) {
        if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&serverAd, &serverLen)) > 0 && strcmp(buffer, "END"))
            write(fd, buffer, num);
    }
    printf("SUCCESS!\n");

    /* Close */
    close(fd);
    close(c);
    // WINDOW ONLY
    WSACleanup();

    return 0;
}