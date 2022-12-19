#include "definitions.h"
#include "http_conn.h"

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

char * http_filePath() {
    char * string = malloc(sizeof(char) * (255 + 1));

    while(true) {
        printf("Put in absolute path to the file to download, starting with /\n");
        if (scanf("%255s", string) != 1) {
            printf("Bad input, please retry");
            continue;
        }
        break;
    }

    return string;
}

char * prepareHttpHeaders(char * serverAddress, char * filePath) {
    //"GET /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    char * headers = malloc(sizeof(char) * (1024));
    strcpy(headers, "GET ");
    strcat(headers, filePath);
    strcat(headers, " HTTP/1.1\r\nHost: ");
    strcat(headers, serverAddress);
    strcat(headers, "\r\n\r\n");

    return headers;
}

int readHttpStatus(int sock) {
    char buffer[1024];
    char * ptr = buffer + 1;
    int receivedBytes, response;

    while((receivedBytes = recv(sock, ptr, 1, 0))){
        if(receivedBytes == -1){
            printf("ERROR: Reading HTTP status.");
            return 0;
        }
        if (*ptr == '\n' && ptr[-1] == '\r')
            break;
        ptr++;
    }

    sscanf(ptr,"%*s %d ", &response);
    printf("HTTP response status: %d\n", response);
    if (receivedBytes > 0)
        return response;
    return 0;
}

int parseHttpHeader(int sock) {
    char buffer[BUFSIZ];
    char * ptr = buffer + 4;
    int receivedBytes;

    while((receivedBytes = recv(sock, ptr, 1, 0))){
        if(receivedBytes == -1){
            printf("ERROR: Parse header.\n");
            return 0;
        }

        if (*ptr == '\n' && ptr[-1] == '\r' && ptr[-2] == '\n' && ptr[-3] == '\r') {
            break;
        }
        ptr++;
    }

    *ptr = 0;
    ptr = buffer + 4;

    if(receivedBytes > 0){
        ptr = strstr(ptr,"Content-Length:");
        if (ptr){
            sscanf(ptr,"%*s %d",&receivedBytes);
        } else {
            receivedBytes = -1;
        }

        printf("HTTP header Content-Length: %d\n", receivedBytes);
    }

    return receivedBytes ;
}

void * http_clientSocket(void * data) {
    HTTP_DATA * httpData = (HTTP_DATA *)data;
    int sock = createSocket(httpData->server, httpData->port);

    char * filePath = http_filePath();
    char * httpRequestHeaders = prepareHttpHeaders(httpData->server->h_name, filePath);
    send(sock, httpRequestHeaders, strlen(httpRequestHeaders), 0);

    if (readHttpStatus(sock) == 0) {
        printf("ERROR: HTTP response status.\n");
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        return NULL;
    }

    int contentLength = parseHttpHeader(sock);
    if (contentLength <= 0) {
        printf("ERROR: HTTP Content-Length.\n");
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        return NULL;
    }

    char * fileName = strrchr(filePath, '/') + 1;

    int receivedBytes, savedBytes = 0;
    char receivedData[1024];
    FILE* file = fopen(fileName,"wb");
    if (!file) {
        printf("Error writing file\n");
        return 0;
    }
    printf("HTTP downloading file: %s.\n", fileName);
    httpData->exit = 1;

    while((receivedBytes = recv(sock, receivedData, 1024, 0))){
        if(receivedBytes == -1){
            printf("ERROR: HTTP receive file.\n");
            break;
        }

        fwrite(receivedData,1,receivedBytes,file);
        savedBytes += receivedBytes;
        if (savedBytes == contentLength)
            break;
    }
    printf("HTTP file successfully downloaded.\n");

    fclose(file);
    logAction(fileName, FTP_CONTROL_PORT);
    close(sock);
    free(httpRequestHeaders);
    free(filePath);

    return NULL;
}