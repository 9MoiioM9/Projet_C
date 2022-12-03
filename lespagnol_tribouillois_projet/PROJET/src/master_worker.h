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

//gestion de lecture / écriture entre le master et les workers
void im_Writing(int tube_write, int nb_prime);
int im_Reading(int m_to_w);

//gestion des ouvertures écriture/lecture des pipes 
int mode_write(int write_pipe[2]);
int mode_read(int read_pipe[2]);

//gestion de la fermerture d'un worker
void closeWorker(int worker_prev, int worker_next, int worker_master);

#endif
