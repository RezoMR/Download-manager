#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>

#include<openssl/bio.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/pem.h>
#include<openssl/x509.h>
#include<openssl/x509_vfy.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct httpData {
    struct hostent *server;
    int port;
    int * sock;
} HTTP_DATA;

void * http_clientSocket(void * data);
char * http_filePath();
char * prepareHttpHeaders(char * serverAddress, char * filePath);
int readHttpStatus(int sock);
int parseHttpHeader(int sock);

#ifdef	__cplusplus
}
#endif
