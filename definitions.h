#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

#define FTP_CONTROL_PORT 21
#define HTTP_PORT 80
#define ALLOWED_DOWNLOADS 3

#define DEBUG false

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct data {
    struct hostent *server;
    int controlPort;
    int dataPort;
    int controlSock;
    int dataSock;

    char * fileName;
    int exit;
    int paused;
    int finished;
    long schedule;

    pthread_mutex_t * logMutex;
    pthread_t * thread;
} DATA;

void printError(char *str);

int level1Choices();
int level0Choices();
int showDownloadsChoices();
int getIntValue();
long getSchedule();

int createSocket(struct hostent * server, int port);
int logAction(char * fileName, int port);
void printLogHistory();
void showDownloads(DATA ** downloads);

struct hostent * createServer();

#ifdef	__cplusplus
}
#endif
