#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master
#define CLE_MASTER  56
#define CLE_CLIENT  54
#define FICHIER "master_client.h"


// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation
void master_to_client(int F_Write, int F_Read, int result);
void client_to_master(int F_Write, int F_Read);



#endif
