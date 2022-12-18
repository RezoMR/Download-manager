#include "client_definitions.h"
#include "k_a_t_definitions.h"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printError("Mandatory parameters: address");
    }
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        printError("ERROR: Server does not exist.");
    }
    FTP_DATA ftpData;
    FTPS_DATA ftpsData;
    HTTP_DATA httpData;
    ftpData.server = server;
    ftpsData.server = server;
    httpData.server = server;

    while (1) {
        int choice = level1_choices();
        pthread_t serviceThread;
        switch (choice) {
            case 1:
                ftpData.controlPort = FTP_CONTROL_PORT;
                pthread_create(&serviceThread, NULL, ftp_control_clientSocket, (void *) &ftpData);
                break;
            case 2:
                ftpsData.controlPort = FTP_CONTROL_PORT;
                pthread_create(&serviceThread, NULL, ftps_control_clientSocket, (void *) &ftpsData);
                break;
            case 3:
                httpData.port = HTTP_PORT;
                pthread_create(&serviceThread, NULL, http_clientSocket, (void *) &httpData);
                break;
            default:
                continue;
        }

        pthread_join(serviceThread, NULL);
    }

    return(EXIT_SUCCESS);
}