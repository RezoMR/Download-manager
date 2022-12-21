#include "definitions.h"
#include "ftp_conn.h"

int ftp_control_choices() {
    int choice;

    printf("Press 1 for GET command\n");
    printf("Press 2 for LIST command\n");
    printf("Press 3 for CD command\n");
    printf("Press 0 for BYE command\n");
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
    DATA * ftpData = (DATA *)data;
    int sock = ftpData->dataSock;

    int	receivedBytes;
    char fileData[1024];
    char * fileName = ftpData->fileName;
    char reply[BUFSIZ];
    char command[256];
    bzero(command, 256);
    bzero(reply, BUFSIZ);

    if (DEBUG)
        printf("Setting binary mode\n");
    write(ftpData->controlSock, "TYPE I\r\n", 8);
    recv(ftpData->controlSock, reply, BUFSIZ, 0);
    if (DEBUG)
        printf("Response %s", reply);
    bzero(command, 256);
    bzero(reply, BUFSIZ);

    strcpy(command, "RETR ");
    strcat(command, fileName);
    strcat(command, "\r\n");
    write(ftpData->controlSock, command, strlen(command));
    recv(ftpData->controlSock, reply, BUFSIZ, 0);
    if (DEBUG)
        printf("Response %s", reply);
    if (atoi(reply) == 200 || atoi(reply) == 150) {
        FILE* file = fopen(fileName,"wb");
        if (!file) {
            printf("Error writing file during FTP file download\n");
        } else {
            while ((receivedBytes = recv(sock, fileData, 1024, 0))) {
                while(ftpData->paused == 1) {
                    sleep(5);
                }
                if (ftpData->finished == 1) {
                    close(sock);
                    fclose(file);
                    return NULL;
                }
                fwrite(fileData, 1, receivedBytes, file);
            }
            fclose(file);
        }
        logAction(fileName, FTP_CONTROL_PORT);
    }
    if (DEBUG)
        printf("File downloaded successfully using FTP protocol.\n");
    return NULL;
}

void ftp_data_list(DATA * data) {
    DATA * ftpData = data;
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

void ftp_data_cwd(DATA * data) {
    DATA * ftpData = data;
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

void ftp_quit(DATA * data) {
    char response[BUFSIZ];
    write(data->controlSock, "QUIT\r\n", 6);
    recv(data->controlSock, response, BUFSIZ, 0);
    if (DEBUG)
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
    DATA * ftpData = (DATA *)data;
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
                ftpData->fileName = ftp_data_fileName();
                pthread_create(&controlThread, NULL, ftp_data_clientSocket, (void *)ftpData);
                ftpData->exit = 1;
                pthread_join(controlThread, NULL);
                break;
            case 2:
                ftp_data_list(ftpData);
                break;
            case 3:
                ftp_data_cwd(ftpData);
                break;
            case 0:
                ftpData->exit = 1;
                break;
            default:
                break;
        }
        if (ftpData->dataSock != -1) {
            if (DEBUG)
                printf("Closing data channel\n");
            close(ftpData->dataSock);
            bzero(response, BUFSIZ);
            recv(sock, response, BUFSIZ, 0);
            if (DEBUG)
                printf("Received response %s\n", response);
        }
        if (ftpData->exit == 1) {
            ftp_quit(ftpData);
            close(sock);
            free(response);
            ftpData->finished = 1;
            return NULL;
        }
    }
}
