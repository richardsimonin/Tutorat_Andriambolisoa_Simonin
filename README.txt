=========================Tutorat Système=========================
-------------Andriambolisoa Stephen & Simonin Richard------------
=================================================================
L'objectif de ce Projet est de comprendre et de parametrer un port USB ainsi qu'une arduino.
Nous disposons d'une carte ainsi que de 6 LED.

Le projet est composé de 3 parties : 
* Le code atmega328.c correspondant à l'ATMega328P se trouve dans le dossier Atmega328p. Si vous souhaitez le charger dans l’Arduino, faites un make all, puis make upload. Attention, cette manipulation n’est possible que si l’ATMega16u2 est “flashé” avec le fichier Arduino-usbserial-uno.hex (accessible dans tous les dossiers);
* La partie concernant l’ATMega16u2 est dans le dossier PAD, lui-même localisé dans le dossier PolytechLille du dossier lufa-LUFA170418. S'y trouvent les fichiers Descriptors.c et PAD.c pour lesquels toute modification implique de flasher à nouveau le programme. Il faut tout d'abord faire un make all, puis court-circuiter le GND et le Reset de l’ATMega16u2 pendant 1 seconde, ensuite effectuer les commandes suivantes depuis le dossier PAD :


### dfu-programmer atmega16u2 erase
### dfu-programmer atmega16u2 flash PAD.hex
### dfu-programmer atmega16u2 reset 
        
Et enfin débrancher puis rebrancher la carte Arduino. Vérifier que l’appareil apparaît bien dans la liste des appareils connectés (faites un lsusb);


* Enfin le PC via le programme pc.c. Compilez-le avec la commande :

### gcc -o ctrl pc.c -lusb-1.0


Il suffit alors de lancer l'exécutable ./ctrl pour obtenir la liste des interfaces, des endpoints entrées/sorties associés à ces interfaces et ensuite accéder à un menu proposant 3 options :
* envoyer une lettre au clavier (en retour on l'affiche pour vérifier le bon envoi) entre “a” et “f” pour éteindre les LEDs et entre “A” et “F” pour les allumer. Le caractère permettant de revenir au menu est le ‘z’;
* lire l'état des boutons. L'appui d'un bouton affiche sur le terminal le nom du dit bouton (selon l’adresse du bouton reçu).
* Arrêter le programme, avec le 3e choix.

Limites : Nous obtenons un segmentation fault (qui n'a pas toujours été là) lors de la compilation du programme pc.c. Nous n'avons pas réussit à le débugger. 
En revanche, les parties ont fonctionné individuelement mais la mise en commun n'est pas une reussite totale.

