#include "definitions.h"
#include "ftp_conn.h"
#include "http_conn.h"
#include "fileManager.c"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    FTP_DATA ftpData;
    HTTP_DATA httpData;

    struct hostent * server;
    int level0Choice, level1Choice, level3Choice;

    while (1) {
        while (ftpData.exit == 0 || httpData.exit == 0) {
            sleep(1);
        }
        pthread_t serviceThread;
        level0Choice = level0Choices();
        switch (level0Choice) {
            case 1:
                server = createServer();
                if (server == NULL)
                    break;

                level1Choice = level1Choices();
                switch(level1Choice) {
                    case 1:
                        ftpData.controlPort = FTP_CONTROL_PORT;
                        ftpData.server = server;
                        ftpData.exit = 0;
                        pthread_create(&serviceThread, NULL, ftp_control_clientSocket, (void *) &ftpData);
                        break;
                    case 2:
                        httpData.port = HTTP_PORT;
                        httpData.server = server;
                        httpData.exit = 0;
                        pthread_create(&serviceThread, NULL, http_clientSocket, (void *) &httpData);
                        break;
                    case 0:
                    default:
                        continue;
                }
                if (DEBUG)
                    pthread_join(serviceThread, NULL);
                break;
            case 2:
                printLogHistory();
                break;

            case 4:
                fileManager();
                break;

            case 0:
                return(EXIT_SUCCESS);

        }
    }
}
