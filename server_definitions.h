#include <pthread.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

void *listen_serverSocket(void *data);
void *ftp_control_serverSocket(void *data);
void *ftp_data_serverSocket(void *data);
void *sftp_serverSocket(void *data);
void *http_serverSocket(void *data);
void *https_serverSocket(void *data);

bool ftp_send_file(int socket, char* file_name);

#ifdef	__cplusplus
}
#endif
