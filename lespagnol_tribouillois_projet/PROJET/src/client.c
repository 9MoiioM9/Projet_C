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

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/


static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<nombre>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombres premiers en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}


/************************************************************************
 * Fonction principale
 ************************************************************************/


void ReponseMaster(int order, int reponse)
{
    switch(order)
    {
        case ORDER_COMPUTE_PRIME : 
            if(reponse == 1)
            {
                printf("Le nombre choisi est premier");
            }else printf("le nombre choisi n'est pas premier");
        break;

        case ORDER_HIGHEST_PRIME : printf("\nLe nombre premier le plus grand calculé est %d\n",reponse);
        break;

        case ORDER_HOW_MANY_PRIME : printf("\n %d ont été calculé\n",reponse);
        break;

        case ORDER_STOP : 
        if(reponse == 1)
        {
            printf("Le master et ses workers se sont bien terminer");
        }else printf("Le master et ses workers ne se sont pas terminer");
        break;

        default : printf("Ordre en attente");
        break;
    }
}


int entree_SC()
{
    key_t cle_client = ftok(FICHIER, CLE_CLIENT);
    myassert(cle_client != -1, "\nPas possible de récupérer la clé\n");

    int sema_mutex = semget(CLE_CLIENT, 1, 0);
    myassert(sema_mutex != -1, "\nIncapable de faire le mutex\n");

    struct sembuf operation = {0, -1 , 0};

    int sema_ope = semop(sema_mutex, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    return sema_mutex;
}

int sortie_SC()
{
    key_t cle_client = ftok(FICHIER, CLE_CLIENT);
    myassert(cle_client != -1, "\nPas possible de récupérer la clé\n");

    int sema_mutex = semget(CLE_CLIENT, 1, 0);
    myassert(sema_mutex != -1, "\nIncapable de récupérer le mutex\n");

    struct sembuf operation = {0, +1 , 0};

    int sema_ope = semop(sema_mutex, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    return sema_mutex;
}


int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);
    printf("%d\n", order); // pour éviter le warning

    
    // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
        // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    // sinon
    //    - entrer en section critique :
    //           . pour empêcher que 2 clients communiquent simultanément
    //           . le mutex est déjà créé par le master
    
    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)

    int sc, master_client, client_master, reponse;
/*
    if(order == ORDER_COMPUTE_PRIME_LOCAL)
    {
        //TODO code thread 
    }else {

        //  gestion SC
        sc = entree_SC();
        
        //  gestion des tubes
        master_client = open("master_client", O_RDONLY);
        myassert(master_client != -1, "ouverture en mode lecture impossible");

        client_master = open("client_master", O_WRONLY);
        myassert(client_master != -1, "ouverture en mode ecriture impossible");
        
    }*/
    master_client = open("master_client", O_RDONLY);
    myassert(master_client != -1, "ouverture en mode lecture impossible");

    client_master = open("client_master", O_WRONLY);
    myassert(client_master != -1, "ouverture en mode ecriture impossible");
    
    int tst = write(client_master, &order, sizeof(int));
    myassert(tst == sizeof(int), "ouais");

    //           . les ouvertures sont bloquantes, il faut s'assurer que
    //             le master ouvre les tubes dans le même ordre
    //    - envoyer l'ordre et les données éventuelles au master

    //    - attendre la réponse sur le second tube
    tst = read(master_client, &reponse, sizeof(int));
    myassert(tst == sizeof(int), "lecture compromise");


    printf("\nReponse : %d\n", reponse);
    /*
    master_client = read("client_master", &reponse, sizeof(int));
    myassert(master_client != sizeof(int), "La lecture est compromise");
    */
    //blocage du master suite à sa réponse
    
    //    - sortir de la section critique

    //sc = sortie_SC();
    
    //    - libérer les ressources (fermeture des tubes, ...)

    close(master_client);
    close(client_master);

    //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
    
    // Une fois que le master a envoyé la réponse au client, il se bloque
    // sur un sémaphore ; le dernier point permet donc au master de continuer
    //
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.
    
    return EXIT_SUCCESS;
}
