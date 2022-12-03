#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct worker
{
    /* data */
    int nb_prime;           //nb premier transmi pour le calcul
    int worker_next;        //pipe vers le prochain worker
    int worker_prev;        //pipe vers le worker précédent
    int worker_master;      //pipe pour envoi de la réponse au master
}worker_data;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[] , worker_data *myworker)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
    //passe 4 arg -> [1]prime, [2]val_pipe master, [3]val_pipe entre worker
    //récupération des arguments avec la fonction atoi(..); transforme chaine de char en int
    
    int nb = atoi(argv[1]);                     //nb premier à tester
    myworker->nb_prime = nb;

    int val_pipe_master = atoi(argv[2]);        //pipe pour parler au master
    myworker->worker_master = val_pipe_master;

    int val_pipe_worker = atoi(argv[3]);        //pipe pour communiquer (ici lecture) 
    myworker->worker_prev = val_pipe_worker;    //avec le worker précédent 

    myworker->worker_next = NO_NEXT ;           //initialisation à NO_NEXT car pas de suivant atm
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(worker_data myworker)
{
    // boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    int value;
    int rep;
    int W_N[2];
    //int M_W[2];
    int W_M[2];
    //int P_W[2];

    while (true)
    {
        value = tube_read(myworker.worker_prev);
        

    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
        if(value == STOP_ORDER)
        {
            if(myworker.worker_next != NO_NEXT) //verif d'un worker suivant faire par rapport à la struct
            {                                   //worker_data.worker_next != NO_NEXT
                myworker.worker_next = test_fonction(W_N, value);   //changer avec la struct worker_data : worker_data.worker_next = worker_next(..)
                close(W_N[1]);                 //close(worker_data.worker_next[1]);
            }else myworker.worker_master = test_fonction(W_N, 1); 

            close(M_W[0]); //?      
            break;

        }else {
            if(value == myworker.nb_prime)      //si valeur = nb du worker alors nb premier
            {
                myworker.worker_master = worker_master(W_M, 1); //envoie 1 pour true
            }else if(value % myworker.nb_prime == 0){     //modulo par rapport au num du worker
                    myworker.worker_master = worker_master(W_M, -1);    //envoie -1 pour false
                }else if(myworker.worker_next != NO_NEXT){
                        myworker.worker_next = worker_next(W_N, value);
                    }else{
                        //créer le worker suiv
                        
                        fork();
                        rep = worker_next(W_N, value);
                    }
        }
    //    sinon c'est un nombre à tester, 4 possibilités :
    //           - le nombre est premier
    //           - le nombre n'est pas premier
    //           - s'il y a un worker suivant lui transmettre le nombre
    //           - s'il n'y a pas de worker suivant, le créer
    }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    worker_data myworker;

    parseArgs(argc, argv, &myworker);     

    
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    //envoie de son numero au master dès sa création
    tube_write(myworker.worker_master, myworker.nb_prime);

    loop(myworker);

    // libérer les ressources : fermeture des files descriptors par exemple

    //fermeture des workers 
    closeWorker(myworker.worker_prev, myworker.worker_next, myworker.worker_master);


    return EXIT_SUCCESS;
}
