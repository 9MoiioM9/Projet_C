#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

static int NB_Prime;

// fonctions éventuelles internes au fichier
/*
void master_to_client(int F_Write, int F_Read, int result)
{
	printf("Début de communication avec le client\n");

	key_t key = ftok(FICHIER, CLE_MASTER);
	assert(key != -1);

	int commande, nb;
	int ret;
	
	
	ret = read(F_Read, &commande, sizeof(int));
	assert(ret == sizeof(int));
	
	if(commande ==  ORDER_COMPUTE_PRIME)
	{
		ret = read(F_Read, &nb, sizeof(int));
		assert(ret == sizeof(int));
	}
	
	ret = write(F_Write, &result, sizeof(int));
	assert(ret == sizeof(int));
	
	printf("résultat transmis : %d \n", result);

}


void client_to_master(int F_Write, int F_Read)
{
	printf("Début de communication avec le master\n");
	
	key_t key = ftok(FICHIER, CLE_CLIENT);
	assert(key != -1);

	int commande,ret, nb;
	scanf("%d", &commande);
	if(commande == 1)
	{
		ret = write(F_Write, &commande, sizeof(int));
		assert(ret == sizeof(int));
		
		printf("\nChoix du nb à tester : ");
		scanf("%d",&nb);
		
		ret = write(F_Write, &nb, sizeof(int));
		assert(ret == sizeof(int));
		
		//attente du resultat 
		bool result;
		
		ret = read(F_Read, &result, sizeof(bool));
		assert(ret == sizeof(bool));
		
		if(result == true) 
		{
			printf("\nLe nombre %d est premier \n"),nb);
		}else printf("\nLe nombre %d n'est pas premier \n"),nb);
		
	}
	ret = read(fdRead, &result, sizeof(int));


	assert(ret == sizeof(int));

}





// fonctions éventuelles proposées dans le .h
*/