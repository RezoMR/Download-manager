#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct httpData {
    struct hostent *server;
    int port;
    int * sock;
    int exit;
} HTTP_DATA;

void * http_clientSocket(void * data);
char * http_filePath();
char * prepareHttpHeaders(char * serverAddress, char * filePath);
int readHttpStatus(int sock);
int parseHttpHeader(int sock);

#ifdef	__cplusplus
}
#endif
