#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

void printError(char *str) {
    if (errno != 0) {
        perror(str);
    }
    else {
        fprintf(stderr, "%s\n", str);
    }
    exit(EXIT_FAILURE);
}

int createSocket(struct hostent * server, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("ERROR: Socket.");
    }

    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);
    if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("ERROR: Connect.");
    }

    return sock;
}

int logAction(char * fileName, int port) {
    FILE* file = fopen("log/history.txt","a+");
    if (!file) {
        printf("Error during logging. Log will be missing, sadly :(");
        return 1;
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char timeString[64];
    strftime(timeString, sizeof(timeString), "%c", tm);

    fprintf(file, "%s %d %s\n", timeString, port, fileName);
    fclose(file);
    return 0;
}

void printLogHistory() {
    FILE * file = fopen("log/history.txt","r");
    if (!file) {
        printf("Error occurred during log reading, can't show saved logs\n");
    } else {
        char string[256];
        printf("\nDownload history:\n");
        while (fgets(string, 256, file) != NULL) {
            printf("%s", string);
        }
        printf("\n");
        fclose(file);
    }
}

int level1Choices() {
    int choice;

    printf("Press 1 for FTP transfer\n");
    printf("Press 2 for HTTP transfer\n");
    printf("Press 0 to go back\n");
    while (1) {
        if (scanf("%d", &choice) == 1)
            break;
        else
            printf("You put in wrong value... please retry\n");
        while (getchar() != '\n')
            continue;
    }

    return choice;
}

int level0Choices() {
    int choice;

    printf("Press 1 to download a file\n");
    printf("Press 2 to show download history\n");
    printf("Press 0 to end application\n");
    while (1) {
        if (scanf("%d", &choice) == 1)
            break;
        else
            printf("You put in wrong value... please retry\n");
        while (getchar() != '\n')
            continue;
    }

    return choice;
}

char * prompt_fileName() {
    char * string = malloc(sizeof(char) * (2048));

    while(1) {
        printf("Put in url (host) to download file from\n");
        if (scanf("%2048s", string) != 1) {
            printf("Bad input, please retry");
            continue;
        }
        break;
    }

    return string;
}

struct hostent * createServer() {
    char * string = prompt_fileName();
    struct hostent * server = gethostbyname(string);
    if (server == NULL) {
        printf("ERROR: Server does not exist.");
    }

    free(string);
    return server;
}


