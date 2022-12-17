# download-manager

Zadanie:
 - Download manažér (2 študenti). Klientska aplikácia, ktorá dovolí používateľovi pripojiť sa na vzdialený server apomocou štandardných protokolov stiahnuť obsah dostupný na serveri. Manažérmusí podporovaťnasledujúce protokoly:•http, https, ftp, ftps,•bittorent (+ 1 študent),pričom umožní:•naplánovať čas, kedy sa má sťahovanie začať,•ukončiť sťahovanie,•spravovať lokálne adresáre,•spravovať históriu sťahovania,•definovať prioritu sťahovania,•vprípade súčasného sťahovania viacerých súborov ich naozaj sťahovať súčasne anie postupne.Pri protokoloch, ktoré to dovoľujú, aplikáciatiež umožní:•pozastaviť alebo obnoviť sťahovaniesúboru.Poznámka:  aplikácia  nesmie  byť  implementovaná  spoužitím  nástroja Wget  ainých  podobných  nástrojov slúžiacich na sťahovanie internetového obsahu.

Aplikacia:
 - Aplikacia sluzi na (asynchronnu) vymenu textovych sprav medzi dvomi pouzivatelmi.
 - Asynchronnost je zabezpecena vyuzitim vlakien a funkcie select.

Preklad servera:
 - gcc k_a_t_definitions.c k_a_t_server.c -o k_a_t_server -pthread
Preklad klienta:
 - gcc k_a_t_definitions.c k_a_t_client.c -o k_a_t_client -pthread

Spustenie servera:
 - ./k_a_t_server 10000 server
Spustenie klienta:
 - ./k_a_t_client localhost 10000 klient

Ukoncenie aplikacie:
 - klient alebo server zada spravu ":end"