#include "server_definitions.h"
#include "k_a_t_definitions.h"

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

void * listen_serverSocket(void * arg) {
    int port = *(int *)arg;
    printf("Listening on port %d\n", port);

    //vytvorenie TCP socketu <sys/socket.h>
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Error: socket.");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu

    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Error: bind.");
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    listen(serverSocket, 10);

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket;
    while ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength)))
    {
        pthread_t listeningThread;
        int * newSocket = malloc(1);
        *newSocket = clientSocket;
        switch (port) {
            case FTP_CONTROL_PORT:
                pthread_create(&listeningThread, NULL,  ftp_control_serverSocket, (void*) newSocket);
            //case SFTP_PORT:
            //    pthread_create(&listeningThread, NULL,  sftp_serverSocket, (void*) newSocket);
            case HTTP_PORT:
                pthread_create(&listeningThread, NULL,  http_serverSocket, (void*) newSocket);
            case HTTPS_PORT:
                pthread_create(&listeningThread, NULL,  https_serverSocket, (void*) newSocket);
        }
        pthread_join(listeningThread, NULL);
        close(clientSocket);
        free(newSocket);
    }
    if (clientSocket < 0)
    {
        printError("Error: accept.");
    }

    return NULL;
}

void * sftp_serverSocket(void * arg) {
    return NULL;
}

void * http_serverSocket(void * arg) {
    return NULL;
}

void * https_serverSocket(void * arg) {
    return NULL;
}

void *ftp_control_serverSocket(void *newSocket)
{
    int socket = *(int *)newSocket;
    char * buffer[10];
    //maybe while loop
    read(socket, buffer, 10);
    if (strcmp(*buffer, "GET") == 0) {
        pthread_t ftpGetThread;
        pthread_create(&ftpGetThread, NULL,  ftp_data_serverSocket, NULL);
        pthread_join(ftpGetThread, NULL);
    }

    return NULL;
}

void *ftp_data_serverSocket(void * data)
{
    char server_response[BUFSIZ], client_request[BUFSIZ], fileName[256];

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Error: socket.");
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    //serverAddress.sin_port = htons(FTP_DATA_PORT);       //nastavenie portu

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Error: bind.");
    }

    listen(serverSocket, 10);

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket;
    while ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength)))
    {
        recv(clientSocket, fileName, BUFSIZ, 0);
        if (access(fileName, F_OK) != -1){
            write(clientSocket, "OK", 3);
            ftp_send_file(clientSocket, fileName);
        }
        else {
            // Requested file does not exist, notify the client
            strcpy(server_response, "NO");
            write(clientSocket, "NO", 3);
        }

        close(clientSocket);
    }
    if (clientSocket < 0)
    {
        printError("Error: accept.");
    }

    return NULL;
}

bool ftp_send_file(int clientSocket, char * fileName)
{
    struct stat	fileStatus;
    int	fileDesc, fileSize;

    stat(fileName, &fileStatus);
    fileDesc = open(fileName, O_RDONLY);
    fileSize = fileStatus.st_size;
    send(clientSocket, &fileSize, sizeof(int), 0);
    sendfile(clientSocket, fileDesc, NULL, fileSize);

    return true;
}
