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

#define FTP_CONTROL_PORT 21
#define HTTP_PORT 80
#define HTTPS_PORT 2443

#ifdef	__cplusplus
extern "C" {
#endif

void printError(char *str);

int level1Choices();
int level0Choices();

int createSocket(struct hostent * server, int port);

struct hostent * createServer();

#ifdef	__cplusplus
}
#endif
