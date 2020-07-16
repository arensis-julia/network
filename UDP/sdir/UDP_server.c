#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
// LINUX:
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// WINDOW:
#include <winsock2.h>
#include <signal.h>
#include "dirent.h"

#define MAX 1024
#define serverPort 8000

int main(int argc, char * argv[]) {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int c;
    SOCKET c;
    struct sockaddr_in serverAd, clientAd;
    struct dirent * entry;
    DIR * flist;
    char * fileName, * filePath;
    char buffer[MAX] = {0, };
    int clientLen;
    int fileNum;
    int num, fd;
    int isNum = 0, flag = 0;
    int i = 0;

    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    /* Creating socket */
    if((c = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Cannot create socket\n");
        exit(-1);
    }

    /* Setting server address */
    memset(&serverAd, 0, sizeof(serverAd));
    serverAd.sin_family = AF_INET;
    serverAd.sin_port = htons(serverPort);
    serverAd.sin_addr.s_addr = htonl(INADDR_ANY);
    clientLen = sizeof(clientAd);
    
    /* Binding */
    if((bind(c, (struct sockaddr *)&serverAd, sizeof(serverAd))) < 0) {
        fprintf(stderr, "[ERROR] Binding failed\n");
        exit(-2);
    }

    /* Receive request FLIST */
    memset(buffer, '\0', MAX);
    if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&clientAd, &clientLen)) < 0) {
        fprintf(stderr, "[ERROR] Receiving FLIST failed\n");
        exit(-3);
    }
    printf("Receive Request: [%s]\n", buffer);
    
    /* Transfer file list */
    if((flist = opendir("./sendingFile")) == NULL) {
        fprintf(stderr, "[ERROR] Opening directory failed\n");
        exit(-4);
    }
    printf("\n====================FILE LIST====================\n");
    while((entry = readdir(flist)) != NULL) {
        if(strcmp(".", entry->d_name) && strcmp("..", entry->d_name)) {
            sendto(c, entry->d_name, strlen(entry->d_name) + 1, 0, (struct sockaddr *)&clientAd, clientLen);
            printf("[%d] %s\n", i, entry->d_name);
            i++;
        }
    }
    closedir(flist);
    printf("==================================================\n\n");
    sendto(c, "END", 4, 0, (struct sockaddr *)&clientAd, clientLen);

    /* Receive file request */
    memset(buffer, '\0', MAX);
    if((num = recvfrom(c, buffer, MAX, 0, (struct sockaddr *)&clientAd, &clientLen)) < 0) {
        fprintf(stderr, "[ERROR] Receiving file request failed\n");
        exit(-5);
    }
    fileName = strdup(buffer);

    for(i=0; i<strlen(fileName); i++) {
        if(fileName[i] < '0' || fileName[i] > '9')
            isNum = 1;
    }
    if(isNum > 0)
        printf("Requested File: %s\n", fileName);
    else {
        fileNum = atoi(fileName);
        printf("Requested File Number: %d\n", fileNum);
    }

    /* Finding file */
    flag = 0;
    i = 0;
    if((flist = opendir("./sendingFile")) == NULL) {
        fprintf(stderr, "[ERROR] Opening directory failed\n");
        exit(-6);
    } 
    while((entry = readdir(flist)) != NULL) {
        if(isNum > 0) {
            if(!strcmp(fileName, entry->d_name)) {
                sendto(c, "VALID", 6, 0, (struct sockaddr *)&clientAd, clientLen);
                flag = 1;
                break;
            }
        }
        else {
            if((i-2) == fileNum) {
                sendto(c, "VALID", 6, 0, (struct sockaddr *)&clientAd, clientLen);
                fileName = entry->d_name;
                flag = 1;
                break;
            }
            i++;
        }
    }
    if(flag == 0) {
        sendto(c, "INVALID", 8, 0, (struct sockaddr *)&clientAd, clientLen);
        printf("Invalid Request\n");
        exit(-7);
    }
    if(isNum == 0) {
        printf("Requested File: %s\n", fileName);
        sendto(c, fileName, strlen(fileName)+1, 0, (struct sockaddr *)&clientAd, clientLen);
    }
    closedir(flist);

    if(flag) {
        filePath = malloc(strlen("./sendingFile") + strlen(fileName) + 1);
        strcpy(filePath, "./sendingFile/");
        strcat(filePath, fileName);
        if((fd = open(filePath, O_RDONLY)) < 0) {
            fprintf(stderr, "[ERROR] File open failed\n");
            exit(-8);
        }
        memset(buffer, '\0', MAX);
        while((num = read(fd, buffer, MAX)) > 0) {
            sendto(c, buffer, num, 0, (struct sockaddr *)&clientAd, clientLen);
        }
        sendto(c, "END", 4, 0, (struct sockaddr *)&clientAd, clientLen);
    }

    /* Close */
    printf("BYE!\n");
    close(fd);
    close(c);
    // WINDOW ONLY
    WSACleanup();

    return 0;
}