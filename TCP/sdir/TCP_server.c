#include <stdio.h>
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
    // LINUX: int c, lis;
    SOCKET c, lis;
    struct sockaddr_in serverAd, clientAd;
    char * fileName;
    char buffer[MAX] = {0, };
    unsigned short serverPort;
    int clientLen;
    // LINUX: int opt = 1;
    char opt = (char)1;
    int fd, num = 0;

    /* Checking arguments */
    if(argc < 3) {
        fprintf(stderr, "[ERROR] Missing arguments: PORT_NUMBER FILE_NAME\n");
        exit(-1);
    }
    serverPort = atoi(argv[1]);
    fileName = argv[2];

    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    /* Creating listening socket */
    if((lis = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Cannot create listening socket\n");
        exit(-2);
    }
    setsockopt(lis, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Setting server address */
    memset(&serverAd, 0, sizeof(serverAd));
    serverAd.sin_family = AF_INET;
    serverAd.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAd.sin_port = htons(serverPort);

    /* Binding */
    if((bind(lis, (const struct sockaddr *)&serverAd, sizeof(serverAd))) < 0) {
        fprintf(stderr, "[ERROR] Binding failed\n");
        exit(-3);
    }

    /* Listening */
    if((listen(lis, 5)) < 0) {
        fprintf(stderr, "[ERROR] Listening failed\n");
        exit(-4);
    }
    printf("Waiting for clients...\n");

    /* Accepting client */
    clientLen = sizeof(clientAd);
    // LINUX: if((c = accept(lis, (struct sockaddr *)&clientAd, (socklen_t *)&clientLen)) < 0) {
    if((c = accept(lis, (struct sockaddr *)&clientAd, (int *)&clientLen)) < 0) {
        fprintf(stderr, "[ERROR] Accepting failed\n");
        exit(-5);
    }
    printf("Connected!\n");

    /* Read file */
    if((fd = open(fileName, O_RDONLY)) < 0) {
        fprintf(stderr, "[ERROR] File open failed\n");
        exit(-6);
    }

    /* Transferring information about file (name, size) */
    // SEND NAME
    send(c, fileName, MAX, 0);
    
    if((num = recv(c, buffer, MAX, 0)) < 0) {
        fprintf(stderr, "[ERROR] Receive failed\n");
        exit(-7);
    }
    if(strcmp(buffer, "ACK")) {
        fprintf(stderr, "[ERROR] No ACK received\n");
        exit(-8);
    }
    memset(buffer, '\0', MAX);

    /* Transferring file to client */
    printf("Send [%s] to the client...\n", fileName);
    memset(buffer, '\0', MAX);
    while((num = read(fd, buffer, MAX)) > 0)
        send(c, buffer, num, 0);
    
    /* Close */
    printf("BYE!\n");
    close(fd);
    close(c);
    // WINDOW ONLY
    WSACleanup();

    return 0;
}