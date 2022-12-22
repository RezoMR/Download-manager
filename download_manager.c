#include "definitions.h"
#include "ftp_conn.h"
#include "http_conn.h"

#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    struct hostent * server;
    int level0Choice, level1Choice;

    DATA ** downloads = malloc(sizeof(DATA) * ALLOWED_DOWNLOADS);
    for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
        downloads[i] = NULL;
    }
    int usedCapacity = 0;

    int performingAction;

    while (1) {
        for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
            if (downloads[i] != NULL && downloads[i]->finished == 1) {
                free(downloads[i]->fileName);
                free(downloads[i]->thread);
                free(downloads[i]);
                downloads[i] = NULL;
                usedCapacity--;
            }
        }

        performingAction = 0;
        for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
            if (downloads[i] != NULL && downloads[i]->exit == 0) {
                performingAction = 1;
                break;
            }
        }
        if (performingAction) {
            sleep(1);
            continue;
        }

        level0Choice = level0Choices();
        switch (level0Choice) {
            case 1:
                if (usedCapacity == ALLOWED_DOWNLOADS) {
                    printf("Allowed downloads exceeded, wait until queue is not empty\n");
                    break;
                }

                server = createServer();
                if (server == NULL)
                    break;

                DATA * data;
                data = malloc(sizeof(DATA));
                pthread_t *serviceThread = malloc(sizeof(pthread_t));
                data->thread = serviceThread;
                data->paused = 0;
                data->finished = 0;
                data->exit = 0;
                data->schedule = 0;
                level1Choice = level1Choices();
                data->server = server;
                switch(level1Choice) {
                    case 1:
                        data->controlPort = FTP_CONTROL_PORT;
                        pthread_create(serviceThread, NULL, ftp_control_clientSocket, (void *) data);
                        break;
                    case 2:
                        data->controlPort = HTTP_PORT;
                        pthread_create(serviceThread, NULL, http_clientSocket, (void *) data);
                        break;
                    default:
                        free(data->thread);
                        free(data);
                        continue;
                }
                for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
                    if (downloads[i] == NULL) {
                        downloads[i] = data;
                        usedCapacity++;
                        break;
                    }
                }
                if (DEBUG)
                    pthread_join(*serviceThread, NULL);
                break;
            case 2:
                printLogHistory();
                break;
            case 3:
                showDownloads(downloads);
                break;
            case 0:
                free(downloads);
                return(EXIT_SUCCESS);

        }
    }
}
