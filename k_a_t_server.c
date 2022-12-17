#include "k_a_t_definitions.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printError("Sever je nutne spustit s nasledujucimi argumentmi: port pouzivatel.");
    }
    int port = atoi(argv[1]);
	if (port <= 0) {
		printError("Port musi byt cele cislo vacsie ako 0.");
	}
    char *userName = argv[2];
    
    //vytvorenie TCP socketu <sys/socket.h>
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Chyba - socket.");        
    }
    
    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu
    
    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - bind.");
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    listen(serverSocket, CLIENTS);

    DATA * multipleData[CLIENTS];
    int availablePlaces[CLIENTS];
    for (int i = 0; i < CLIENTS; i++) {
        availablePlaces[i] = 1;
    }
//    time_t inactivityTime = -1; //-1 if threads still running

    CLEANUP_DATA cleanupData;
    cleanupData.multipleData = multipleData;
    cleanupData.availablePlaces = availablePlaces;
    cleanupData.clean = 1;

    pthread_t cleanupThread;
    pthread_create(&cleanupThread, NULL, data_cleanup, (void *) &cleanupData);

    int availablePlace;
    while (1) {
        availablePlace = -1;
        for (int i = 0; i < CLIENTS; i++) {
            if (availablePlaces[i] == 1) {
                availablePlace = i;
                break;
            }
        }
        if (availablePlace == -1) {
            printf("Nie je volne miesto na pripojenie, cakam dokial sa uvolni miesto...");
            sleep(3);
            continue;
        }

        //server caka na pripojenie klienta <sys/socket.h>
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
        if (clientSocket > 0) {
            //inicializacia dat zdielanych medzi vlaknami
            DATA data;
            data_init(&data, userName, clientSocket);

            cleanupData.availablePlaces[availablePlace] = 0;
            cleanupData.multipleData[availablePlace] = &data;

            //vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
            pthread_t thread;
            pthread_create(&thread, NULL, data_writeData, (void *) &data);
            //v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
            pthread_t threadRead;
            pthread_create(&threadRead, NULL, data_readData, (void *) &data);
        }
//        bool inactive = true;

//        if (inactive) {
//            if (inactivityTime == -1) {
//                printError("A");
//                inactivityTime = time(NULL);
//            } else {
//                printf("%0.2f", difftime(time(NULL), inactivityTime));
//                if (difftime(time(NULL), inactivityTime) >= INACTIVITY_TIMEOUT) {
//                    break;
//                }
//            }
//        }
    }

    cleanupData.clean = 0;
    //uzavretie pasivneho socketu <unistd.h>
    close(serverSocket);
    return (EXIT_SUCCESS);
}