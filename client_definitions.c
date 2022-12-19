#include "client_definitions.h"
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

int level1_choices() {
    int choice;

    printf("Press 1 for FTP transfer\n");
    printf("Press 2 for FTPS transfer\n");
    printf("Press 3 for HTTP transfer\n");
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

int ftp_control_choices() {
    int choice;

    printf("Press 0 for BYE command\n");
    printf("Press 1 for GET command\n");
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

char * ftp_data_fileName() {
    char * string = malloc(sizeof(char) * (255 + 1));

    while(true) {
        printf("Put in filename to download\n");
        if (scanf("%255s", string) != 1) {
            printf("Bad input, please retry");
            continue;
        }
        break;
    }

    return string;
}

void * ftp_data_clientSocket(void * data) {
    FTP_DATA * ftpData = (FTP_DATA *)data;
    int port = ftpData->dataPort;

    int	file_size, file_desc;
    char * fileData;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("ERROR: Socket.");
    }

    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)ftpData->server->h_addr, (char *)&serverAddress.sin_addr.s_addr, ftpData->server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("ERROR: Connect.");
    }

    char * fileName = ftp_data_fileName();
    char reply[BUFSIZ];
    char command[256];

    strcpy(command, "RETR ");
    strcat(command, fileName);
    strcat(command, "\r\n");
    write(*ftpData->controlSock, command, strlen(command));
    recv(*ftpData->controlSock, reply, BUFSIZ, 0);
    printf("Response %s", reply);
    if (atoi(reply) == 200) {
        recv(sock, &file_size, sizeof(int), 0);
        fileData = malloc(file_size);
        file_desc = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
        recv(sock, fileData, file_size, 0);
        write(file_desc, fileData, file_size);
        close(file_desc);
        free(fileData);
    }
    else {
        printf("Bad request\n");
    }

    free(fileName);

    return NULL;
}

int ftp_login(const int * socket) {
    char username[256];
    char password[256];
    printf("Put in username: ");
    while (1) {
        if (scanf("%255s", username) != 1) {
            printf("\n");
            printf("Bad input, please retry\n");
            continue;
        }
        break;
    }

    char response[256];
    char command[256];
    strcpy(command, "USER ");
    strcat(command, username);
    strcat(command, "\n");
    write(*socket, command, strlen(command));
    recv(*socket, response, 256, 0);
    printf("Username login response: %d\n", atoi(response));
    if (atoi(response) != 331) {
        return 1;
    }

    printf("Put in password: ");
    while (1) {
        if (scanf("%255s", password) != 1) {
            printf("\n");
            printf("Bad input, please retry\n");
            continue;
        }
        break;
    }

    bzero(command, strlen(command));
    bzero(response, 255);
    strcpy(command, "PASS ");
    strcat(command, password);
    strcat(command, "\n");
    write(*socket, command, strlen(command));
    recv(*socket, &response, 255, 0);
    printf("Password login response: %d\n", atoi(response));
    if (atoi(response) != 230) {
        return 1;
    }

    return 0;
}

int ftp_data_extractPort(char * string) {
    char * result = strtok(string, "(");
    result = strtok(NULL, ")");
    strtok(result, ",");
    strtok(NULL, ",");
    strtok(NULL, ",");
    strtok(NULL, ",");
    char * firstValue = strtok(NULL, ",");
    char * secondValue = strtok(NULL, ",");

    int portNumber = atoi(firstValue) * 256 + atoi(secondValue);

    return portNumber;
}

void * ftp_control_clientSocket(void * data) {
    FTP_DATA * ftpData = (FTP_DATA *)data;
    int sock = createSocket(ftpData->server, ftpData->controlPort);
    ftpData->controlSock = &sock;

    char response[256];
    recv(sock, response, 256, 0);
    printf("Connection response: %d\n", atoi(response));

    int loginResult = ftp_login(&sock);
    if (loginResult == 1) {
        printf("Login unsucessful\n");
        return NULL;
    }
    printf("Login sucessful\n");

    int value;
    pthread_t controlThread;
    bzero(response, 256);
    while (1) {
        value = ftp_control_choices();
        switch (value) {
            case 1:
                write(sock, "PASV\n", 6);
                recv(sock, response, 256, 0);
                printf("Received response %s", response);
                ftpData->dataPort= ftp_data_extractPort(response);
                pthread_create(&controlThread, NULL, ftp_data_clientSocket, (void *)ftpData);
                pthread_join(controlThread, NULL);
                break;
            case 0:
                close(sock);
                return NULL;
            default:
                break;
        }
    }
}

SSL * createSSL() {
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    BIO* certbio = BIO_new(BIO_s_file());
    BIO* outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

    if (SSL_library_init() < 0) {
        printf("ERROR: OpenSSL initialization.\n");
        return NULL;
    }
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx)
        printf("ERROR: SSL context creation.\n");
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    SSL* ssl = SSL_new(ctx);

    return ssl;
}

void * ftps_data_clientSocket(void * data) {
    FTPS_DATA * ftpsData = (FTPS_DATA *)data;
    int port = ftpsData->dataPort;

    int	file_size, file_desc;
    char * fileData;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("ERROR: Socket.");
    }

    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)ftpsData->server->h_addr, (char *)&serverAddress.sin_addr.s_addr, ftpsData->server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("ERROR: Connect.");
    }

    char * fileName = ftp_data_fileName();
    char reply[BUFSIZ];
    char command[256];

    strcpy(command, "RETR ");
    strcat(command, fileName);
    strcat(command, "\r\n");
    SSL_write(ftpsData->controlSock, command, strlen(command));
    SSL_read(ftpsData->controlSock, reply, BUFSIZ);
    printf("Response %s", reply);
    if (atoi(reply) == 200) {
        recv(sock, &file_size, sizeof(int), 0);
        fileData = malloc(file_size);
        file_desc = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
        recv(sock, fileData, file_size, 0);
        write(file_desc, fileData, file_size);
        close(file_desc);
        free(fileData);
    }
    else {
        printf("Bad request\n");
    }

    free(fileName);

    return NULL;
}

void * ftps_control_clientSocket(void * data) {
    FTPS_DATA * ftpsData = (FTPS_DATA *)data;
    int sock = createSocket(ftpsData->server, ftpsData->controlPort);
    SSL * ssl = createSSL();
    SSL_set_fd(ssl, sock);
    ftpsData->controlSock = ssl;

    if (SSL_connect(ssl) < 1) {
        printf("ERROR: SSL connect session.");
    }

    X509* cert = SSL_get_peer_certificate(ssl);

    if (!cert) {
        printf("ERROR: Retrieving certificate from server.");
    }

    //X509_NAME* certname = X509_get_subject_name(cert);

    char response[256];
    SSL_read(ssl, response, 256);
    printf("Connection response: %d\n", atoi(response));

    int loginResult = ftp_login(&sock);
    if (loginResult == 1) {
        printf("Login unsucessful\n");
        return NULL;
    }
    printf("Login sucessful\n");

    int value;
    pthread_t controlThread;
    bzero(response, 256);
    while (1) {
        value = ftp_control_choices();
        switch (value) {
            case 1:
                SSL_write(ssl, "PASV\n", 6);
                SSL_read(ssl, response, 256);
                printf("Received response %s", response);
                ftpsData->dataPort= ftp_data_extractPort(response);
                pthread_create(&controlThread, NULL, ftps_data_clientSocket, (void *)ftpsData);
                pthread_join(controlThread, NULL);
                break;
            case 0:
                close(sock);
                return NULL;
            default:
                break;
        }
    }
}

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
    *ptr = 0;
    ptr = buffer + 1;
    printf("%s", ptr);

    sscanf(ptr,"%*s %d ", &response);
    printf("HTTP response status: %d\n", response);
    printf("Received bytes: %d\n", receivedBytes);
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
    printf("%s\n", httpRequestHeaders);

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
        printf("Error writing file");
        return 0;
    }
    printf("HTTP downloading file: %s.\n", fileName);

    while((receivedBytes = recv(sock, receivedData, 1024, 0))){
        if(receivedBytes == -1){
            printf("ERROR: HTTP receive file.");
            break;
        }

        fwrite(receivedData,1,receivedBytes,file);
        savedBytes += receivedBytes;
        if (savedBytes == contentLength)
            break;
    }

    fclose(file);
    close(sock);
    free(httpRequestHeaders);
    free(filePath);

    return NULL;
}
