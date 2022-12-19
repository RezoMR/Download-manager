#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include<openssl/bio.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/pem.h>
#include<openssl/x509.h>
#include<openssl/x509_vfy.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct ftpData {
    struct hostent *server;
    int controlPort;
    int dataPort;
    int controlSock;
    int dataSock;
    char * fileName;
    int exit;
} FTP_DATA;

typedef struct ftpsData {
    struct hostent *server;
    int controlPort;
    int dataPort;
    SSL * controlSock;
    SSL * dataSock;
} FTPS_DATA;

int ftp_control_choices();
char * ftp_data_fileName();
char * ftp_cwd_path();
int ftp_login(const int *socket);
int ftp_data_extractPort(char * string);

char * recvData(int socket);

void * ftp_control_clientSocket(void * data);
void * ftp_data_clientSocket(void * data);
void ftp_data_list(FTP_DATA * data);
void ftp_data_cwd(FTP_DATA * data);
void ftp_quit(FTP_DATA * data);
void * ftps_control_clientSocket(void * data);
void * ftps_data_clientSocket(void * data);
SSL * createSSL();

#ifdef	__cplusplus
}
#endif
