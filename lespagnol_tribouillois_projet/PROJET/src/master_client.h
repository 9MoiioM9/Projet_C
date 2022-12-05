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


//Gestion de nos constantes
#define CLE_CLIENT  54  //clé pour la gestion du mutex

#define FICHIER "master_client.h" 

#define PIPE_MASTER_TO_CLIENT   "master_client" //tube nommé de master vers client
#define PIPE_CLIENT_TO_MASTER   "client_master" //tube nommé de client vers client

//ORDER_COMPUTE_PRIME
#define INIT_VALUE  0
#define NO_PRIME    0   //si le nombre est pas premier alors 0
#define IS_PRIME    1   //si le nombre est premier alors 1



// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation

int entree_SC();

int sortie_SC();

int lecture_nb(int tube);

void * codeThread(void * arg);
void compute_prime_local(int number);


#endif
