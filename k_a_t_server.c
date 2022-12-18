#include "k_a_t_definitions.h"
#include "server_definitions.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pthread.h>

int main2(int argc, char* argv[]) {
    printf("Zaciatok servera\n");
    int ftp = FTP_CONTROL_PORT;
    //int sftp = SFTP_PORT;
    int http = HTTP_PORT;
    int https = HTTPS_PORT;
    pthread_t ftpThread, sftpThread, httpThread, httpsThread;
    pthread_create(&ftpThread, NULL, listen_serverSocket, (void *)&ftp);
    //pthread_create(&sftpThread, NULL, listen_serverSocket, (void *)&sftp);
    pthread_create(&httpThread, NULL, listen_serverSocket, (void *)&http);
    pthread_create(&httpsThread, NULL, listen_serverSocket, (void *)&https);

    pthread_join(ftpThread, NULL);
    pthread_join(sftpThread, NULL);
    pthread_join(httpThread, NULL);
    pthread_join(httpsThread, NULL);

    return (EXIT_SUCCESS);
}