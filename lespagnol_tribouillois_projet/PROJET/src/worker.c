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
//#include <wait.h>

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
    int myValue, ret;   //myValue pour le nb envoyer par le master
                        //ret pour vérifier le bon fonctionnement de certaine création

    while (true)
    {
        myValue = tube_read(myworker.worker_prev);
        

    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
        if(myValue == STOP_ORDER)
        {
            if(myworker.worker_next != NO_NEXT) //verif d'un worker suivant faire par rapport à la struct
            {                                   //worker_data.worker_next != NO_NEXT
                im_Writing(myworker.worker_next, myValue);   //changer avec la struct worker_data : worker_data.worker_next = worker_next(..)
                /*
                system d'attente c le premier worker qui se fermera en dernier !
                //wait(NULL);
                */
            }else im_Writing(myworker.worker_master, 1); 

            //si le worker n'a pas de suivant alors on sort de la boucle 
            //pour pouvoir supprimer ce dernier !  
            break;

        }else {
            if(myValue == myworker.nb_prime)      //si valeur = nb du worker alors nb premier
            {
                im_Writing(myworker.worker_master, 1); //envoie 1 pour true
            }else if(myValue % myworker.nb_prime == 0){     //modulo par rapport au num du worker
                    im_Writing(myworker.worker_master, -1);    //envoie -1 pour false
                }else if(myworker.worker_next != NO_NEXT){
                        im_Writing(myworker.worker_next, myValue);
                    }else{
                        //création d'un nouveau pipe pour la communication avec le nouveau worker
                        int new_pipe_for_worker[2];
                        //juste pour vérifier la création du pipe, on sait jamais 
                        ret = pipe(new_pipe_for_worker); 
                        myassert(ret != -1, "creation du nouveau pipe compromise");

                        //même fonctionnement que dans master.c 
                        pid_t ret_fork = fork();
                        myassert(ret_fork != -1, "nouveau fork mal construit");
                        if(ret_fork == 0){ //on accède au fils = 0
                            int new_read_for_new_worker = mode_read(new_pipe_for_worker); 
                            //création du worker suivant 
                            worker_creation(myValue, myworker.worker_master, new_read_for_new_worker); 
                        }
                        //changement au niveau du tube worker_next
                        //création d'une nouvelle valeur pour pas créer bug avec le fils
                        int new_next_worker = mode_write(new_pipe_for_worker);
                        myworker.worker_next = new_next_worker;
                    }
        }
    }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    worker_data myworker;

    parseArgs(argc, argv, &myworker);     

    //On envoie 1 au master dès sa création pour confirmation d'un nombre premier
    //sachant qu'un worker est créer alors c'est forcément un nombre premier !
    tube_write(myworker.worker_master, 1);

    loop(myworker);

    //fermeture des workers 
    closeWorker(myworker.worker_prev, myworker.worker_next, myworker.worker_master);


    return EXIT_SUCCESS;
}
