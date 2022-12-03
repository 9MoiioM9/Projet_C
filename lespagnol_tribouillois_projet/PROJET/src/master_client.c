#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

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

// fonctions éventuelles internes au fichier

int entree_SC(int key)
{

    key_t cle_client = ftok(FICHIER, key);
    myassert(cle_client != -1, "\nPas possible de récupérer la clé\n");

    int sema_mutex = semget(key, 1, 0);
    myassert(sema_mutex != -1, "\nIncapable de faire le mutex\n");

    struct sembuf operation = {0, -1 , 0};

    int sema_ope = semop(sema_mutex, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    return sema_mutex;
}

int sortie_SC(int key)
{
    key_t cle_client = ftok(FICHIER, key);
    myassert(cle_client != -1, "\nPas possible de récupérer la clé\n");

    int sema_mutex = semget(key, 1, 0);
    myassert(sema_mutex != -1, "\nIncapable de récupérer le mutex\n");

    struct sembuf operation = {0, +1 , 0};

    int sema_ope = semop(sema_mutex, &operation, 1);
    myassert(sema_ope != -1 , "\nOpération invalide\n");

    return sema_mutex;
}





// fonctions éventuelles proposées dans le .h
