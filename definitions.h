#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

#define FTP_CONTROL_PORT 21
#define HTTP_PORT 80

#define DEBUG false

#ifdef	__cplusplus
extern "C" {
#endif

void printError(char *str);

int level1Choices();
int level0Choices();

int createSocket(struct hostent * server, int port);
int logAction(char * fileName, int port);
void printLogHistory();

struct hostent * createServer();

#ifdef	__cplusplus
}
#endif
