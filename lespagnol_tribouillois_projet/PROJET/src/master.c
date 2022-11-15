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
    assert(ret == sizeof(int));

    close(envoi[1]);

    exit(EXIT_FAILURE);
}

bool worker_to_master(int rep[2])
{

    bool res;
    close(rep[1]);
    int ret;

    ret = read(rep[0], &res, sizeof(bool));
    assert(ret == sizeof(bool));

    close(rep[0]);

    return res; 
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(/* paramètres */)
{
    // boucle infinie :
    // - ouverture des tubes (cf. rq client.c)
    int master_client = open("master_client", O_WRONLY);
    int client_master = open("client_master", O_RDONLY);
    int envoi[2];
    int resp[2];
    pid_t retFork;
    
    int commande;
    int nbr_test;
    bool retour;
    // - attente d'un ordre du client (via le tube nommé)
    // - si ORDER_STOP
    if(commande == -1)
    {
        // demader au worker de se fermer
    }
    //       . envoyer ordre de fin au premier worker et attendre sa fin
    //       . envoyer un accusé de réception au client
    // - si ORDER_COMPUTE_PRIME
    //       . récupérer le nombre N à tester provenant du client
    //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
    //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
    //             on leur envoie tous les nombres entre M+1 et N-1
    //             note : chaque envoie déclenche une réponse des workers
    //       . envoyer N dans le pipeline
    retFork = fork();
    assert(retFork != -1);

    master_to_worker(envoi, nbr_test);
    //       . récupérer la réponse
    worker_to_master(resp, retour);
    //       . la transmettre au client
    if(commande == 1)
    {

    }
    // - si ORDER_HOW_MANY_PRIME
    //       . transmettre la réponse au client
   // if(// à chaque fois qu'un worker renvoie true
  //  {
   // 	nb_prime++;
   // }
    // - si ORDER_HIGHEST_PRIME
    //       . transmettre la réponse au client
    int max = 0;
    int result;
    if(max < result)
    {
    	max = result;
    }
    // - fermer les tubes nommés
    close(master_client);
    close(client_master);
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
    // - revenir en début de boucle
    //
    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
     if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores
    int sema1, sema2;
    int mutex;
    /*
    sema1 = semget(, 2, IPC_CREAT | 0641);
    assert(sema1 != -1);
    
    sema2 = semget(, 2, IPC_CREAT | 0641);
    assert(sema2 != -1);
    */
    // - création des tubes nommés
    int ret1 = mkfifo("master_client", 0644);
    assert(ret1 == 0);
    int ret2 = mkfifo("client_master", 0644);
    assert(ret2 == 0);
    
    // - création du premier worker


    // boucle infinie
    loop(/* paramètres */);

    // destruction des tubes nommés, des sémaphores, ...
    
    ret1 = unlink("master_client");
    assert(ret1 == 0);
    ret2 = unlink("client_master");
    assert(ret2 == 0);

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
