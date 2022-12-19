#include "definitions.h"
#include "ftp_conn.h"

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

int ftp_control_choices() {
    int choice;

    printf("Press 0 for BYE command\n");
    printf("Press 1 for GET command\n");
    printf("Press 2 for LIST command\n");
    printf("Press 3 for CD command\n");
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

char * recvData(int socket) {
    char * buffer = malloc(sizeof(char) * BUFSIZ);
    char * ptr = buffer;
    char statusCode[3];
    int hasChar = 0;
    int multiline = 0;
    int index = 0;

    while(recv(socket, ptr, 1, 0)) {
        if (*ptr == '\n' && ptr[-1] == '\r')
            if (multiline == 0) {
                break;
            }

        if (isalpha(*ptr)) {
            hasChar = 1;
        }
        if (*ptr == '-' && hasChar == 0) {
            multiline = 1;
            strncpy(statusCode, buffer, 3);
        }

        if (index > 4 && multiline == 1 && *ptr == statusCode[2] && ptr[-1] == statusCode[1] && ptr[-2] == statusCode[0] && ptr[-3] == '\n' && ptr[-4] == '\r')
            multiline = 0;
        ptr++;
        index++;
    }

    return buffer;
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

char * ftp_cwd_path() {
    char * string = malloc(sizeof(char) * (255 + 1));

    while(true) {
        printf("Put in directory name to move to\n");
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
    int sock = ftpData->dataSock;

    int	file_size, receivedBytes;
    char fileData[1024];
    char * fileName = ftp_data_fileName();
    char reply[BUFSIZ];
    char command[256];
    bzero(command, 256);
    bzero(reply, BUFSIZ);

    printf("Setting binary mode\n");
    write(ftpData->controlSock, "TYPE I\r\n", 8);
    recv(ftpData->controlSock, reply, BUFSIZ, 0);
    printf("Response %s", reply);
    bzero(command, 256);
    bzero(reply, BUFSIZ);

    strcpy(command, "RETR ");
    strcat(command, fileName);
    strcat(command, "\r\n");
    write(ftpData->controlSock, command, strlen(command));
    recv(ftpData->controlSock, reply, BUFSIZ, 0);
    printf("Response %s", reply);
    if (atoi(reply) == 200 || atoi(reply) == 150) {
        FILE* file = fopen(fileName,"wb");
        if (!file) {
            printf("Error writing file\n");
        } else {
            //recv(sock, &file_size, sizeof(int), 0);
            //write(file_desc, fileData, file_size);
            while ((receivedBytes = recv(sock, fileData, 1024, 0))) {
                fwrite(fileData, 1, receivedBytes, file);
            }
            fclose(file);
        }
    }

    free(fileName);
    return NULL;
}

void ftp_data_list(FTP_DATA * data) {
    FTP_DATA * ftpData = data;
    int sock = ftpData->dataSock;

    char reply[BUFSIZ];
    char command[256];

    bzero(command, 256);
    bzero(reply, BUFSIZ);
    strcpy(command, "LIST\r\n");
    write(ftpData->controlSock, command, strlen(command));
    recv(ftpData->controlSock, reply, BUFSIZ, 0);
    printf("Response %s\n", reply);
    if (atoi(reply) == 150) {
        recv(sock, &reply, sizeof(reply), 0);
        printf("LIST reply:\n%s\n", reply);
    }
    else {
        printf("Bad request\n");
    }
}

void ftp_data_cwd(FTP_DATA * data) {
    FTP_DATA * ftpData = data;
    char command[256];

    char * dir = ftp_cwd_path();
    bzero(command, 256);
    strcpy(command, "CWD ");
    strcat(command, dir);
    strcat(command, "\r\n");
    write(ftpData->controlSock, command, strlen(command));
    char * reply = recvData(ftpData->controlSock);
    printf("Response %s\n", reply);

    free(reply);
    free(dir);
}

void ftp_quit(FTP_DATA * data) {
    char response[BUFSIZ];
    write(data->controlSock, "QUIT\r\n", 6);
    recv(data->controlSock, response, BUFSIZ, 0);
    printf("Received response %s\n", response);
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
    strcat(command, "\r\n");
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
    strcat(command, "\r\n");
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
    ftpData->controlSock = sock;

    char * response = malloc(sizeof(char) * BUFSIZ);
    recv(sock, response, BUFSIZ, 0);
    printf("Connection response: %d\n", atoi(response));

    int loginResult = ftp_login(&sock);
    if (loginResult == 1) {
        printf("Login unsucessful\n");
        return NULL;
    }
    printf("Login sucessful\n");

    int value;
    pthread_t controlThread;
    bzero(response, BUFSIZ);
    while (1) {
        bzero(response, BUFSIZ);
        value = ftp_control_choices();

        ftpData->dataSock = -1;
        if (value == 1 || value == 2) {
            printf("Establishing data channel\n");
            write(sock, "PASV\r\n", 6);
            recv(sock, response, BUFSIZ, 0);
            printf("Received response %s\n", response);
            ftpData->dataPort = ftp_data_extractPort(response);
            ftpData->dataSock = createSocket(ftpData->server, ftpData->dataPort);
        }

        switch (value) {
            case 1:
                pthread_create(&controlThread, NULL, ftp_data_clientSocket, (void *)ftpData);
                pthread_join(controlThread, NULL);
                break;
            case 2:
                ftp_data_list(ftpData);
                break;
            case 3:
                ftp_data_cwd(ftpData);
                break;
            case 0:
                ftp_quit(ftpData);
                close(sock);
                free(response);
                return NULL;
            default:
                break;
        }
        if (ftpData->dataSock != -1) {
            printf("Closing data channel\n");
            close(ftpData->dataSock);
            bzero(response, BUFSIZ);
            recv(sock, response, BUFSIZ, 0);
            printf("Received response %s\n", response);
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