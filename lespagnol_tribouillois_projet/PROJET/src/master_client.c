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

static int NB_Prime;

// fonctions éventuelles internes au fichier


// fonctions éventuelles proposées dans le .h