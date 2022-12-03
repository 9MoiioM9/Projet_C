#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)


//Gestion de nos constantes 
#define NO_NEXT     0  //si pas encore de worker suivant alors 0 jusqu'à la création d'un suivant
#define STOP_ORDER  -1  //ORDRE STOP emis par le client   

//Gestion des foncions utiles entre worker/master

void worker_creation(int myPrime, int worker_to_master, int myPrevious_Worker);

int worker_master(int envoi[2], int res);
int master_worker(int envoi[2]);
int worker_next(int envoi[2], int nbr_prime);
int prev_worker(int envoi[2]);

#endif
