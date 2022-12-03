#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_worker.h"

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

int master_worker(int envoi[2]){

    close(envoi[1]);
    int ord;
    int m_w;

    m_w = read(envoi[0], &ord, sizeof(int));
    myassert(ord == sizeof(int),"pobleme de lecture d'un ordre");

    close(envoi[0]);

    return ord;
}

int worker_master(int envoi[2], int res){

    close(envoi[0]);

    int ret = write(envoi[1], &res, sizeof(int));
    myassert(ret == sizeof(int),"probleme du retour du worker");

    return envoi[1];
}

int worker_next(int envoi[2], int nbr_prime){

    close(envoi[0]);

    int ret = write(envoi[1], &nbr_prime, sizeof(int));
    myassert(ret == sizeof(int),"probleme ériture prime au work suiv");

    return envoi[1];
}
int prev_worker(int envoi[2]){

    close(envoi[1]);
    int nb_prime, p_w;

    p_w = read(envoi[0], &nb_prime, sizeof(int));
    myassert(nb_prime == sizeof(int),"problème lecture du prime");

    close(envoi[0]);

    return nb_prime;
}

//renvoie un tube anonyme seulement en lecture après avoir fermé l'écriture
int mode_read(int read_pipe[2]){

    int is_read = close(read_pipe[1]);
    myassert(is_read != -1, "tube d'écriture mal fermé ");

    return read_pipe[0];
}
//de même pour le tue anonyme d'écriture
int mode_write(int write_pipe[2]){

    int is_write = close(write_pipe[0]);
    myassert(is_write != -1, "tube de lecture mal fermé ");
    
    return write_pipe[1];
}
// fonctions éventuelles proposées dans le .h
