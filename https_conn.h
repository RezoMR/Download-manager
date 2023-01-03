#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef	__cplusplus
extern "C" {
#endif

void * https_clientSocket(void * data);
int readHttpsStatus(SSL * ssl);
int parseHttpsHeader(SSL * ssl);

int initializeSSL();
void destroySSL();

#ifdef	__cplusplus
}
#endif
