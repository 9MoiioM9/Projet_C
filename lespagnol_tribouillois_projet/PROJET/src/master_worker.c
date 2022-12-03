#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_worker.h"

#include <unistd.h>

// fonctions éventuelles internes au fichier
void worker_creation(int myPrime, int worker_to_master, int myPrevious_Worker){

    char *argv[4];  //pointeur pour l'exec

    //chaine pour mettre dans argv pour l'exec du nouveau worker
    char nb_prime[5];
    char master_worker[5];
    char worker_master[5];

    //remplissage des chaines avec valeurs reçues en paramètre
    sprintf(nb_prime, "%d",myPrime);
    sprintf(master_worker, "%d", myPrevious_Worker);
    sprintf(worker_master, "%d", worker_to_master);

    //On complète maintenant argv avec nos chaines contenant nos valeurs de paramètre
    argv[0] = "worker";         //pour le nom dans le exec
    argv[1] = nb_prime;         //notre valeur pour le worker à sa creation
    argv[2] = worker_master;    //le tube suivant
    argv[3] = master_worker;    //le tube précédent 
    //On garde l'ordre par rapport à la fonction parseArgs !

    //On peut enfin créer notre worker avec un exec
    exec(argv[0], argv);
    //Gestion d'erreur au cas ou si l'exec disfonctionne 
    //TODO erreur

}

//Fonction qui lit le résultat venant soit de l'un des worker ou du master
int im_Reading(int m_to_w){ //pipe via la structure de worker 

    int nb_test;
    int m_w;

    m_w = read(m_to_w, &nb_test, sizeof(int));
    myassert(nb_test == sizeof(int),"probleme de lecture d'un ordre");

    return nb_test;
}

int test_fonction(int test[2], int val){
    close(test[0]);

    int ret = write(test[1], &val, sizeof(int));
    myassert(ret != -1, "probleme de transmission");

    return test[1];
}


//Fonction qui transmet soit le nb à tester au worker suivant, soit le résultat au master
void im_Writing(int tube_write, int nb_prime){     //pipe et nb premier via struct worker

    int ret = write(tube_write, &nb_prime, sizeof(int));
    myassert(ret == sizeof(int),"probleme ériture prime au work suiv");
}

//renvoie un tube anonyme seulement en lecture après avoir fermé l'écriture
int mode_read(int read_pipe[2]){

    int is_read = close(read_pipe[1]);
    myassert(is_read != -1, "tube d'écriture mal fermé ");

    return read_pipe[0];
}
//Inversement pour le tube anonyme d'écriture
int mode_write(int write_pipe[2]){

    int is_write = close(write_pipe[0]);
    myassert(is_write != -1, "tube de lecture mal fermé ");

    return write_pipe[1];
}

//gestion fermeture dans l'ordre des tubes 
void closeWorker(int worker_prev, int worker_next, int worker_master){
    
    int close_worker;
    close_worker = close(worker_prev);

    //on vérifie qu'il existe un suivant avant
    if (worker_next != NO_NEXT){
        close_worker = close(worker_next);
        myassert(close_worker != -1, "le worker_next ne s'est pas close");
    }

    //une fois que tous les workers sont bien fermés 
    //on fini enfin par le pipe entre master/worker
    close_worker = close(worker_master);
    myassert(close_worker != -1, "Le pipe entre master/worker ne sais pas close");
}
// fonctions éventuelles proposées dans le .h
