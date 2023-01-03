#include "definitions.h"
#include "ftp_conn.h"
#include "http_conn.h"
#include "https_conn.h"
#include "fileManager.h"

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
    pthread_mutex_t logMutex;
    pthread_mutex_init(&logMutex, NULL);

    int sslConn = initializeSSL();

    while (1) {
        for (int i = 0; i < ALLOWED_DOWNLOADS; i++) {
            if (downloads[i] != NULL && downloads[i]->finished == 1) {
                if (downloads[i] != NULL)
                    free(downloads[i]->fileName);
                pthread_join(*downloads[i]->thread, NULL); // do this step to avoid memory leaks
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
                data->logMutex = &logMutex;

                data->paused = 0;
                data->finished = 0;
                data->exit = 0;
                data->schedule = 0;

                level1Choice = level1Choices();
                data->server = server;
                data->fileName = NULL;
                switch(level1Choice) {
                    case 1:
                        data->controlPort = FTP_CONTROL_PORT;
                        pthread_create(serviceThread, NULL, ftp_control_clientSocket, (void *) data);
                        break;
                    case 2:
                        data->controlPort = HTTP_PORT;
                        pthread_create(serviceThread, NULL, http_clientSocket, (void *) data);
                        break;
                    case 3:
                        if (sslConn) {
                            printf("SSL initialization failed, try restarting application, or don't use methods that require SSL\n");
                            break;
                        }
                        data->controlPort = HTTPS_PORT;
                        pthread_create(serviceThread, NULL, https_clientSocket, (void *) data);
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
                pthread_mutex_lock(&logMutex);
                printLogHistory();
                pthread_mutex_unlock(&logMutex);
                break;
            case 3:
                showDownloads(downloads);
                break;
            case 4:
                fileManager();
                break;
            case 0:
                if (usedCapacity > 0) {
                    printf("Downloads are still pending, cancel them or wait for them to finish before exiting the application\n");
                    break;
                }
                pthread_mutex_destroy(&logMutex);
                free(downloads);
                destroySSL();
                return(EXIT_SUCCESS);
        }
    }
}
