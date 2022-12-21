#include "definitions.h"
#include "http_conn.h"

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
    bzero(buffer, 1024);
    char * ptr = buffer + 1;
    int receivedBytes, response;

    while((receivedBytes = recv(sock, ptr, 1, 0))){
        if(receivedBytes == -1){
            printf("ERROR: Reading HTTP status\n");
            return 0;
        }
        if (*ptr == '\n' && ptr[-1] == '\r')
            break;
        ptr++;
    }

    ptr = buffer + 1;
    strtok(ptr, " ");
    char * result = strtok(NULL, " ");
    response = atoi(result);
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
            printf("ERROR: Parse header\n");
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
    DATA * httpData = (DATA *)data;
    int sock = createSocket(httpData->server, httpData->controlPort);

    char * filePath = http_filePath();
    char * httpRequestHeaders = prepareHttpHeaders(httpData->server->h_name, filePath);
    char * fileName = malloc(sizeof(char) * 256);
    fileName = strcpy(fileName, strrchr(filePath, '/') + 1);
    httpData->fileName = fileName;
    send(sock, httpRequestHeaders, strlen(httpRequestHeaders), 0);

    if (readHttpStatus(sock) != 200) {
        printf("ERROR: HTTP response status\n");
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        httpData->finished = 1;
        return NULL;
    }

    int contentLength = parseHttpHeader(sock);
    if (contentLength <= 0) {
        printf("ERROR: HTTP Content-Length.\n");
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        httpData->finished = 1;
        return NULL;
    }

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
        if(receivedBytes == -1 || httpData->finished == 1){
            if (DEBUG)
                printf("ERROR: HTTP receive file.\n");
            fclose(file);
            close(sock);
            free(httpRequestHeaders);
            free(filePath);

            return NULL;
        }

        fwrite(receivedData,1,receivedBytes,file);
        savedBytes += receivedBytes;
        if (savedBytes == contentLength)
            break;
    }
    if (DEBUG)
        printf("HTTP file successfully downloaded.\n");

    fclose(file);
    logAction(fileName, HTTP_PORT);
    close(sock);
    free(httpRequestHeaders);
    free(filePath);
    httpData->finished = 1;

    return NULL;
}