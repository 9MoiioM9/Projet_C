#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>  //mkfifo 
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

#include "errno.h"

extern int errno ;


/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

void master_to_worker(int envoi[2], int nbr)
{
    close(envoi[0]);
    int ret;

    ret = write(envoi[1], &nbr, sizeof(int));
    myassert(ret == sizeof(int),"Ecriture comprosie");

    close(envoi[1]);

    exit(EXIT_FAILURE);
}

bool worker_to_master(int rep[2])
{

    bool res;
    close(rep[1]);
    int ret;

    ret = read(rep[0], &res, sizeof(bool));
    myassert(ret == sizeof(bool), "Lecture compromise");

    close(rep[0]);

    return res;
}

/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(/* paramètres */)
{
    int commande, nb_test, sc, ret, master_client, client_master;
    int r = 1;
    int howmany = 5;
    int val_test = 0;

    int errnum;
    // int max = 0;
    // int result;
    // if(max < result)
    // {
    //     max = result;
    // }

    // boucle infinie :
    while (true){
        
        printf("\nJ'attends\n");
        
        val_test++;
        // - ouverture des tubes (cf. rq client.c)
        master_client = open(PIPE_MASTER_TO_CLIENT, O_WRONLY);
        myassert(master_client != -1, "le tube master vers client ne s'est pas ouvert");
        
        client_master = open(PIPE_CLIENT_TO_MASTER, O_RDONLY);
        myassert(client_master != -1, "le tube client vers master ne s'est pas ouvert");

        printf("\nOrdre numero : %d\n", val_test);

        int envoi[2];
        int resp[2];
        
       
        // - attente d'un ordre du client (via le tube nommé)

        ret = read(client_master, &commande, sizeof(int));
        myassert(ret == sizeof(int), "lecture compromise");
       
        printf("\nordre reçu est : %d \n", commande);
       
        //===============================================================================================
        // - si ORDER_STOP
        
        if(commande == -1){
            printf("\n STOP\n");

            ret = write(master_client, &r, sizeof(int)); //r = 1 pour true 
            myassert(ret == sizeof(int), "lecture compromise");
        }else {
    
            printf("\nReponse : %d\n", howmany);


            printf("\nECRITURE DE LA REPONSE\n");
            ret = write(master_client, &howmany, sizeof(int));
            myassert(ret == sizeof(int), "ecriture compromise");
        
        //===============================================================================================
        // - si ORDER_COMPUTE_PRIME
            if(commande == 1)
            {
                int retFork = fork();
                if(retFork == 0)
                {
                    //exec à faire
                }
                myassert(retFork != -1, "problème au niveau du fork pour les workers");

                master_to_worker(envoi, nb_test);
                // récupérer la réponse
                int retour = worker_to_master(resp);
                //la transmettre au client
                ret = write(master_client, &retour, sizeof(int));
                myassert(ret == sizeof(int), "ecriture compromise");
            }
        }

        //===============================================================================================
        // - si ORDER_HIGHEST_PRIME
        //       . transmettre la réponse au client

        sleep(3); //evite le pb de priorité avec le client 
        
        // - fermer les tubes nommés
        printf("\n FERMETURE DES TUBES MASTER VERS CLIENT ET INVERSE\n");
        close(master_client);
        close(client_master);

        // - attendre ordre du client avant de continuer (sémaphore : précédence)
        // - revenir en début de boucle

        if(commande == -1){
            printf("\n LE MASTER VA BREAK\n "); 
            break;
        }
        
    }

}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
     if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores

    int sema_precedence = semget(CLE_MASTER, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(sema_precedence != -1, "le semaphore ne s'est pas creer");
    
    struct sembuf operation = {0, +1 , 0};

    int sema_ope = semop(sema_precedence, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    int sema_mutex = semget(CLE_CLIENT, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(sema_mutex != -1, "le semaphore ne s'est pas creer");

    sema_ope = semop(sema_mutex, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    // - création des tubes nommés

    int tube_mc = mkfifo(PIPE_MASTER_TO_CLIENT, 0644);
    myassert(tube_mc != -1, "problème au niveau du tube master vers client");
    int tube_cm = mkfifo(PIPE_CLIENT_TO_MASTER, 0644);
    myassert(tube_cm != 1, "problème au niveau du tube client vers master");
    
    // - création du premier worker


    // boucle infinie
    loop();

    // destruction des tubes nommés, des sémaphores, ...
    
    int delete_sema = semctl(sema_mutex, -1, IPC_RMID);
    myassert(delete_sema != -1, "la semaphore mutex s'est mal détruite");

    delete_sema = semctl(sema_precedence, -1, IPC_RMID);
    myassert(delete_sema != -1, "la semaphore précédence s'est mal détruite");


    tube_mc = unlink(PIPE_MASTER_TO_CLIENT);
    myassert(tube_mc != -1, "Le tube 1 ne s'est pas bien détruit");
    tube_cm = unlink(PIPE_CLIENT_TO_MASTER);
    myassert(tube_cm != -1, "le tube 2 ne s'est pas détruit correctement");

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien


// demander au worker de se fermer
        //       . envoyer ordre de fin au premier worker et attendre sa fin
        //       . envoyer un accusé de réception au client



        //===============================================================================================
        // - si ORDER_COMPUTE_PRIME
        //       . récupérer le nombre N à tester provenant du client
        //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
        //             il faut connaître le plus nombre (M) déjà envoyé aux workers
        //             on leur envoie tous les nombres entre M+1 et N-1
        //             note : chaque envoie déclenche une réponse des workers
        //       . envoyer N dans le pipeline

        //TODO 
        

        
        //===============================================================================================
        // - si ORDER_HOW_MANY_PRIME
        //       . transmettre la réponse au client