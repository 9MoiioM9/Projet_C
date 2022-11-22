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

#include "master_client.h"
#include "master_worker.h"

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
    myassert(ret == sizeof(int),"reuighiez");

    close(envoi[1]);

    exit(EXIT_FAILURE);
}

bool worker_to_master(int rep[2])
{

    bool res;
    close(rep[1]);
    int ret;

    ret = read(rep[0], &res, sizeof(bool));
    myassert(ret == sizeof(bool), "zkeogzoi");

    close(rep[0]);

    return res;
}

/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(/* paramètres */)
{
    // boucle infinie :
    while (true){
        // - ouverture des tubes (cf. rq client.c)
        int master_client = open("master_client", O_WRONLY);
        myassert(master_client != -1, "le tube master_client ne s'est pas ouvert");

        int client_master = open("client_master", O_RDONLY);
        myassert(client_master != -1, "le tube client_master ne s'est pas ouvert");
        
    
        int envoi[2];
        int resp[2];
        
        int commande, nb_test,sc;
        // - attente d'un ordre du client (via le tube nommé)

        

        int ret = read(client_master, &commande, sizeof(int));
        myassert(ret == sizeof(int), "lecture compromise");
            printf("test d'ordre : %d ", commande);
       


        //===============================================================================================
        // - si ORDER_STOP
        int r = 1;
        if(commande == -1)
        {
            ret = write(master_client, &r, sizeof(int));
            myassert(ret == sizeof(int), "lecture compromise");
        }
        // demader au worker de se fermer
        //       . envoyer ordre de fin au premier worker et attendre sa fin
        //       . envoyer un accusé de réception au client



        //===============================================================================================
        // - si ORDER_COMPUTE_PRIME
        //       . récupérer le nombre N à tester provenant du client
        //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
        //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
        //             on leur envoie tous les nombres entre M+1 et N-1
        //             note : chaque envoie déclenche une réponse des workers
        //       . envoyer N dans le pipeline
        if(commande == 1)
        {
            int retFork = fork();
            if(retFork == 0)
            {
            
            }
            myassert(retFork != -1, "problème au niveau du fork pour les workers");

            master_to_worker(envoi, nb_test);
        //       . récupérer la réponse
            int retour = worker_to_master(resp);
        //       . la transmettre au client
            ret = write(master_client, &retour, sizeof(int));
            myassert(ret == sizeof(int), "ecriture compromise");
        }
        
        //===============================================================================================
        // - si ORDER_HOW_MANY_PRIME
        //       . transmettre la réponse au client
        int howmany = 0;

        printf("\nReponse : %d\n", howmany);

        sc = entree_SC(56);
        ret = write(master_client, &howmany, sizeof(int));
        myassert(ret == sizeof(int), "ecriture compromise");

        sc = sortie_SC(56);
    

        //===============================================================================================
        // - si ORDER_HIGHEST_PRIME
        //       . transmettre la réponse au client
        
        int max = 0;
        int result;
        if(max < result)
        {
            max = result;
        }
        */
        // - fermer les tubes nommés
        printf("\nIICIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIii\n");
        close(master_client);
        close(client_master);
        // - attendre ordre du client avant de continuer (sémaphore : précédence)
        // - revenir en début de boucle
        //
        // il est important d'ouvrir et fermer les tubes nommés à chaque itération
        // voyez-vous pourquoi ?
        if(commande == -1){
            break;
        }
    }
    
}

/*
int reponseMaster(){
    key_t cle_master = ftok(FICHIER, CLE_MASTER);
    myassert(cle_master != -1, "\nPas possible de récupérer la clé\n"); 

    int sema_rep = semget(CLE_MASTER, 1, 0);
    myassert(sema_rep != -1, "\nIncapable de faire le mutex\n");

    struct sembuf operation = {0, -1 , 0};

    int sema_ope = semop(sema_rep, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    return sema_rep;
}
*/


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

    int tube_mc = mkfifo("master_client", 0644);
    myassert(tube_mc != -1, "problème au niveau du tube master_client");
    int tube_cm = mkfifo("client_master", 0644);
    myassert(tube_cm != 1, "problème au niveau du tube client_master");
    
    // - création du premier worker


    // boucle infinie
    loop();

    // destruction des tubes nommés, des sémaphores, ...
    

    tube_mc = unlink("master_client");
    myassert(tube_mc != -1, "Le tube 1 ne s'est pas bien détruit");
    tube_cm = unlink("client_master");
    myassert(tube_cm != -1, "le tube 2 ne s'est pas détruit correctement");

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien

