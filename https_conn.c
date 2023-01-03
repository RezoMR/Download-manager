#include "definitions.h"
#include "https_conn.h"
#include "http_conn.h"

int initializeSSL() {
    int ssl_init_result = SSL_library_init();
    if (ssl_init_result < 0) {
        printf("ERROR: Failed to initialize SSL connection");
        return 1;
    }
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    return 0;
}

void destroySSL() {
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}

int readHttpsStatus(SSL * ssl) {
    char buffer[1024];
    bzero(buffer, 1024);
    char * ptr = buffer + 1;
    int receivedBytes, response;

    while((receivedBytes = SSL_read(ssl, ptr, 1))){
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
    if (DEBUG)
        printf("HTTP response status: %d\n", response);
    if (receivedBytes > 0)
        return response;
    return 0;
}

int parseHttpsHeader(SSL * ssl) {
    char buffer[BUFSIZ];
    char * ptr = buffer + 4;
    int receivedBytes;

    while((receivedBytes = SSL_read(ssl, ptr, 1))){
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
        if (DEBUG)
            printf("HTTP header Content-Length: %d\n", receivedBytes);
    }

    return receivedBytes ;
}

void * https_clientSocket(void * data) {
    DATA * httpData = (DATA *)data;

    char * filePath = http_filePath();
    int sock = createSocket(httpData->server, httpData->controlPort);

    //initialize ssl connection
    SSL_CTX * ctx = SSL_CTX_new(TLS_client_method());
    if(ctx == NULL)
    {
        printf("ERROR: CTX failed\n");
        //ERR_print_errors_fp(stderr);
        return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    if(ssl == NULL)
    {
        printf("ERROR: SSL failed\n");
        //ERR_print_errors_fp(stderr);
        return NULL;
    }
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);
    //

    char * httpRequestHeaders = prepareHttpHeaders(httpData->server->h_name, filePath);
    char * fileName = malloc(sizeof(char) * 256);
    httpData->fileName = fileName;

    SSL_write(ssl, httpRequestHeaders, strlen(httpRequestHeaders));

    int httpStatus = readHttpsStatus(ssl);
    if (httpStatus != 200) {
        printf("ERROR: HTTP response status\n");
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        httpData->finished = 1;
        return NULL;
    }
    fileName = strcpy(fileName, strrchr(filePath, '/') + 1);
    printf("HTTP response status: %d\n", httpStatus);

    httpData->schedule = getSchedule();
    httpData->exit = 1;
    while (time(NULL) < httpData->schedule) {
        if (httpData->finished == 1) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            free(httpRequestHeaders);
            free(filePath);
            httpData->schedule = 0;
            return NULL;
        }
        sleep(1);
    }
    httpData->schedule = 0;

//    SSL_write(ssl, httpRequestHeaders, strlen(httpRequestHeaders));
//
//    if (readHttpsStatus(ssl) != 200) {
//        printf("ERROR: HTTP response status\n");
//        SSL_shutdown(ssl);
//        SSL_CTX_free(ctx);
//        close(sock);
//        free(httpRequestHeaders);
//        free(filePath);
//        httpData->finished = 1;
//        return NULL;
//    }

    int contentLength = parseHttpsHeader(ssl);
    if (contentLength <= 0) {
        printf("ERROR: HTTP Content-Length\n");
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
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
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);
        free(httpRequestHeaders);
        free(filePath);
        return NULL;
    }
    if (DEBUG)
        printf("HTTP downloading file: %s.\n", fileName);

    while((receivedBytes = SSL_read(ssl, receivedData, 1024))){
        while(httpData->paused == 1) {
            sleep(5);
        }
        if(receivedBytes < 1 || httpData->finished == 1){
            if (DEBUG)
                printf("ERROR: HTTP receive file.\n");
            httpData->finished = 1;
            fclose(file);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
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
    pthread_mutex_lock(httpData->logMutex);
    logAction(fileName, HTTPS_PORT);
    pthread_mutex_unlock(httpData->logMutex);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
    free(httpRequestHeaders);
    free(filePath);
    httpData->finished = 1;

    return NULL;
}