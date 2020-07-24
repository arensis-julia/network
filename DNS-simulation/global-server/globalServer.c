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
#define globalPort 9000

int main() {
    // WINDOW ONLY
    WSADATA wsaData;
    // LINUX: int lis, c;
    SOCKET lis, c;
    struct sockaddr_in globalServerAd;
    struct sockaddr_in globalClientAd;
    char * domainName;
    char * pos, * tmp;
    char * corIP;
    char buff[MAX] = {0, };
    int globalServerLen, globalClientLen;
    int fd, num;
    // LINUX: int opt = 1;
    char opt = (char)1;
    int flag = 0;

    /* Listening socket */
    // WINDOW ONLY
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        fprintf(stderr, "[ERROR] WSAStartup failed with %d\n", WSAGetLastError());

    if((lis = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "[ERROR] Listening socket failed\n");
        exit(-1);
    }
    setsockopt(lis, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Server address */
    memset(&globalServerAd, 0, sizeof(globalServerAd));
    globalServerAd.sin_family = AF_INET;
    globalServerAd.sin_addr.s_addr = htonl(INADDR_ANY);
    globalServerAd.sin_port = htons(globalPort);
    globalServerLen = sizeof(globalServerAd);

    /* Bind */
    if(bind(lis, (struct sockaddr *)&globalServerAd, globalServerLen) < 0) {
        fprintf(stderr, "[ERROR] Binding failed\n");
        exit(-2);
    }

    /* Listen */
    if(listen(lis, 5) < 0) {
        fprintf(stderr, "[ERROR] Listening failed\n");
        exit(-3);
    }

    /* Accept */
    globalClientLen = sizeof(globalClientAd);
    if((c = accept(lis, (struct sockaddr *)&globalClientAd, &globalClientLen)) < 0) {
        fprintf(stderr, "[ERROR] Accepting failed\n");
        exit(-4);
    }

    /* Receive domain name */
    recv(c, buff, MAX, 0);
    domainName = strdup(buff);
    printf("Domain name received: %s\n", domainName);

    /* Verify domain name */
    pos = strrchr(domainName, '.');
    pos += 1;
    flag = 1;

    /* EDU */
    if(!strcmp(pos, "edu")) {
        /* Open and read file */
        if((fd = open("global_edu_dns.txt", O_RDONLY)) < 0) {
            fprintf(stderr, "[ERROR] Opening global_edu_dns.txt failed\n");
            exit(-5);
        }
        memset(buff, '\0', MAX);
        if((num = read(fd, buff, sizeof(buff))) < 0) {
            fprintf(stderr, "[ERROR] Reading global_edu_dns.txt failed\n");
            exit(-6);
        }

        /* Get result */
        if((tmp = strstr(buff, domainName))) {
            printf("Found IP address!\n");
            strtok(tmp, " \n");
            corIP = strtok(NULL, " \n");
            send(c, corIP, strlen(corIP)+1, 0);
            flag = 0;
            printf("Send result: %s\n", corIP);
        }
        else    flag = 1;
    }

    /* COM */
    else if(!strcmp(pos, "com")) {
        /* Open and read file */
        if((fd = open("global_com_dns.txt", O_RDONLY)) < 0) {
            fprintf(stderr, "[ERROR] Opening global_com_dns.txt failed\n");
            exit(-7);
        }
        memset(buff, '\0', MAX);
        if((num = read(fd, buff, sizeof(buff))) < 0) {
            fprintf(stderr, "[ERROR] Reading global_com_dns.txt failed\n");
            exit(-8);
        }

        /* Get result */
        if((tmp = strstr(buff, domainName))) {
            printf("Found IP address!\n");
            strtok(tmp, " \n");
            corIP = strtok(NULL, " \n");
            send(c, corIP, strlen(corIP)+1, 0);
            flag = 0;
            printf("Send result: %s\n", corIP);
        }
        else    flag = 1;
    }

    /* Cannot find domain name */
    if(flag) {
        send(c, "Not found", 10, 0);
        printf("Send result: Not found\n");
    }

    close(fd);
    close(c);

    return 0;
}