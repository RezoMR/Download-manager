﻿# download-manager

Zadanie:
 - Download manažér (2 študenti). Klientska aplikácia, ktorá dovolí používateľovi pripojiť sa na vzdialený server apomocou štandardných protokolov stiahnuť obsah dostupný na serveri. Manažérmusí podporovaťnasledujúce protokoly:•http, https, ftp, ftps,•bittorent (+ 1 študent),pričom umožní:•naplánovať čas, kedy sa má sťahovanie začať,•ukončiť sťahovanie,•spravovať lokálne adresáre,•spravovať históriu sťahovania,•definovať prioritu sťahovania,•vprípade súčasného sťahovania viacerých súborov ich naozaj sťahovať súčasne anie postupne.Pri protokoloch, ktoré to dovoľujú, aplikáciatiež umožní:•pozastaviť alebo obnoviť sťahovaniesúboru.Poznámka:  aplikácia  nesmie  byť  implementovaná  spoužitím  nástroja Wget  ainých  podobných  nástrojov slúžiacich na sťahovanie internetového obsahu.

 - build: make
 - clean: make clean
 - run: ./download_manager

Testovanie aplikacie:
 - FTP
   - ftp.debian.org
   - mirror.lnx.sk
 - HTTP
   - mirror.dkm.cz
     - /debian-cd/11.6.0/amd64/iso-bd/debian-edu-11.6.0-amd64-BD-1.iso (bigger)
     - /debian-cd/current/amd64/iso-cd/debian-11.6.0-amd64-netinst.iso (medium)
     - /debian-cd/11.6.0/amd64/iso-bd/HEADER.html (smaller)
   - mirror.lnx.sk
     - /www/arch.png
