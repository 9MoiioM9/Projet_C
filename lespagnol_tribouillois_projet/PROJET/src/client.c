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

//fonction pour afficher la réponse du master selon le nb obtenu
void ReponseMaster(int order, int reponse)
{
    switch(order)
    {
        case ORDER_COMPUTE_PRIME : 
            if(reponse == IS_PRIME)
            {
                printf("\nLe nombre choisi est premier\n");
            }else printf("\nLe nombre choisi n'est pas premier\n");
        break;

        case ORDER_HIGHEST_PRIME : printf("\nLe nombre premier le plus grand calculé est %d\n",reponse);
        break;

        case ORDER_HOW_MANY_PRIME : printf("\n %d nombre(s) premier(s) ont été calculé\n",reponse);
        break;

        case ORDER_STOP : 
        if(reponse == 1)
        {
            printf("\nLe master et ses workers se sont bien terminer\n");
        }else printf("\nLe master et ses workers ne se sont pas terminer\n");
        break;

        default : printf("Ordre en attente");
        break;
    }
}


int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);
    printf("verif de l'ordre : %d\n", order); // pour éviter le warning
    
    int sc, master_client, client_master;

    //Gestion de ORDER_COMPUTE_PRIME_LOCAL
    
    //=======================================================================
    //entrée en section critique pour faire sa requête au master
    sc = entree_SC(54);
    myassert(sc != -1, "pb sema");

    //Ouverture de la lecture de l'ordre et écriture de la réponse
    //=======================================================================    
    if(order != 4){
    
        master_client = open(PIPE_MASTER_TO_CLIENT, O_RDONLY);
        myassert(master_client != -1, "ouverture en mode lecture impossible");

        client_master = open(PIPE_CLIENT_TO_MASTER, O_WRONLY);
        myassert(client_master != -1, "ouverture en mode ecriture impossible");
        
        int ret = write(client_master, &order, sizeof(int));
        myassert(ret != -1, "pb d'envoie d'ordre");

        if(order == ORDER_COMPUTE_PRIME){
            //si calcul d'un premier alors on lui transmet le nombre reçu par le user
            ret = write(client_master, &number, sizeof(int));
            myassert(ret != -1, "pb d'envoie d'ordre");
        }
    //=======================================================================
     //éviter le blocage de ressource au niveau de la section critique

        int reponse = lecture_nb(master_client);

        ReponseMaster(order, reponse);

        close(master_client);
        close(client_master);
    }

    //Une fois la réponse obtenue via le master on libère la ressource 
    //Pour sortir de la section critique afin de laisser la place aux autres clients 
    sc = sortie_SC(54);

    return EXIT_SUCCESS;
}

    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    // sinon
    //    - entrer en section critique :
    //           . pour empêcher que 2 clients communiquent simultanément
    //           . le mutex est déjà créé par le master
    
    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)


/*
    if(order == ORDER_COMPUTE_PRIME_LOCAL)
    {
        //TODO code thread 
    }else { */