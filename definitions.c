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
        if (DEBUG)
            printf("Error during logging. Log will be missing, sadly :(\n");
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

int showDownloadsChoices() {
    printf("Press 1 to show running downloads\n");
    printf("Press 2 cancel a download\n");
    printf("Press 3 to pause a download\n");
    printf("Press 4 to resume a download\n");
    printf("Press 0 to go back\n");

    return getIntValue();
}

int getIntValue() {
    int choice;

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

long getSchedule() {
    printf("Press 1 if you want to schedule your download\n");
    printf("Press 0 if you don't want to schedule your download\n");
    int value = getIntValue();
    if (value != 1)
        return 0;

    int hour, minute, second, futureTime = 0;
    printf("Now you should put in timer values, after which your download will start\n");
    printf("Put in number of HOURS to wait until download starts:\n");
    hour = getIntValue();
    printf("Put in number of MINUTES to wait until download starts:\n");
    minute = getIntValue();
    printf("Put in number of SECONDS to wait until download starts:\n");
    second = getIntValue();

    futureTime = second + (60 * minute) + (60 * 60 * hour);

    return time(NULL) + futureTime;
}

void showDownloads(DATA ** downloads) {
    int id;

    while(1) {
        int choice = showDownloadsChoices();
        switch (choice) {
            case 1:
                for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
                    if (downloads[i] != NULL) {
                        char * state;
                        if (downloads[i]->schedule > 0)
                            state = "SCHEDULED";
                        else if (downloads[i]->paused == 0)
                            state = "RUNNING";
                        else
                            state = "PAUSED";
                        printf("ID: %d; PROTOCOL: %d; NAME: %s STATE: %s\n", i, downloads[i]->controlPort,
                               downloads[i]->fileName, state);
                    }
                }
                break;
            case 2:
                printf("Put in ID of a download to cancel\n");
                id = getIntValue();

                if (downloads[id] == NULL)
                    printf("Download with this ID does not exist!\n");
                else {
                    downloads[id]->finished = 1;
                }
                break;
            case 3:
                printf("Put in ID of a download to pause\n");
                printf("(WARNING: HTTP download session breaks after a while of being stopped)\n");
                id = getIntValue();

                if (downloads[id] == NULL)
                    printf("Download with this ID does not exist!\n");
                else if (downloads[id]->paused == 1)
                    printf("Download with this ID is already paused!\n");
                else {
                    downloads[id]->paused = 1;
                }
                break;
            case 4:
                printf("Put in ID of a download to resume\n");
                id = getIntValue();

                if (downloads[id] == NULL)
                    printf("Download with this ID does not exist!\n");
                else if (downloads[id]->paused == 0)
                    printf("Download with this ID is not paused!\n");
                else {
                    downloads[id]->paused = 0;
                }
                break;
            default:
                break;
        }
        break;
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
    printf("Press 3 to show actions for running downloads\n");
    printf("Press 4 to manage Directories and Files\n");
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
        printf("ERROR: Server does not exist\n");
    }

    free(string);
    return server;
}


