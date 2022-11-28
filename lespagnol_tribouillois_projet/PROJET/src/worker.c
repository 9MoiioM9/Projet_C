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

static void parseArgs(int argc, char * argv[] /*, structure à remplir*/)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
}

int master_worker(int envoi[2])
{
    close(envoi[1]);
    int ord;
    int m_w;

    m_w = read(envoi[0], &ord, sizeof(int));
    myassert(ord == sizeof(int),"pobleme de lecture d'un ordre");

    close(envoi[0]);

    return ord;

    exit(EXIT_FAILURE);
}

int woker_master(int envoi[2], int nbr)
{
    close(envoi[0]);

    int ret = write(envoi[1], &nbr, sizeof(int));
    myassert(ret == sizeof(int),"probleme du retour du worker");

    return envoi[1];

}

int worker_next(int envoi[2], int nbr)
{
    close(envoi[0]);

    int ret = write(envoi[1], &nbr, sizeof(int));
    myassert(ret == sizeof(int),"probleme transmission ordre work suiv");

    return envoi[1];
}

int prev_worker(int envoi[2])
{
    close(envoi[1]);
    int nbr;
    int p_w;

    p_w = read(envoi[0], &nbr, sizeof(int));
    myassert(nbr == sizeof(int),"pobleme transmission nb test ");

    close(envoi[0]);

    return nbr;
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(/* paramètres */)
{
    // boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    int ord;
    int rep;
    int ret = 1; // retour d'un ordre au master
    int next = -1;
    int W_N[2];
    int M_W[2];
    int W_M[2];
    int P_W[2];

    while (true)
    {
        ord = master_worker;
        

    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
        if(ord == -1)
        {
            if(next != -1)
            {
                next = worker_next(W_N, ord);
                close(W_N[1]);
            }
            else rep = woker_master(W_M, ret);
            close(M_W[0]);
            break;

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
    parseArgs(argc, argv /*, structure à remplir*/);

    
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    loop(/* paramètres */);

    // libérer les ressources : fermeture des files descriptors par exemple

    return EXIT_SUCCESS;
}
