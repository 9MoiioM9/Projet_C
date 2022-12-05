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
#include <unistd.h> //pour execv
#include <math.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

#include "errno.h"


/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin
typedef struct master{

    int master_to_worker[2];    //création des tubes anonymes de communication 
    int worker_to_master[2];    //entre le master et les workers
    int highest_prime;          //nb premier le plus élevé à mettre à jour dès que besoin 
    int howmany_prime;          //nb de calcul de nb premier (incrémenté à chaque fois) 
    int lecture;                //lecture des msg plus simple si dans la structure et moins long
    int ecriture;               //écriture des msg

}master_data;

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

/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(master_data myMaster)
{
    int command,ret, master_client, client_master;
    int r = 1;
    int val_test = 0;

    // boucle infinie :
    while (true){
        
        printf("\nWelcome in the Waiting Room ...\n");
        
        val_test++;
        //gestion d'ouverture des tubes nommés entre master/client
        //en respectant l'ordre d'ouverture dans client et master
        sleep(2);
        master_client = open(PIPE_MASTER_TO_CLIENT, O_WRONLY);
        myassert(master_client != -1, "le tube master vers client ne s'est pas ouvert");
        
        client_master = open(PIPE_CLIENT_TO_MASTER, O_RDONLY);
        myassert(client_master != -1, "le tube client vers master ne s'est pas ouvert");

        printf("\nClient : %d\n", val_test);


        // - attente d'un ordre du client (via le tube nommé)
        //récupération de l'ordre du client afin de différencier les actions 
        command = lecture_nb(client_master);

        if(command == ORDER_STOP){
            printf("\n STOP\n");

            ret = write(master_client, &r, sizeof(int)); //r = 1 pour true 
            myassert(ret != -1, "écriture compromise");
            //TODO faire la gestion de commande pour fermer les workers

        }else if(command == ORDER_COMPUTE_PRIME) {
            //on récupère la valeur à tester 
            printf("\nOrdre Compute Prime");
            int result, nb_test;
            nb_test = lecture_nb(client_master);
            

            printf("\n\nCalcule du nombre %d\n\n",nb_test);

            //on compare avec le plus haut actuel pour savoir si on connais pas déjà la réponse
            if(nb_test > myMaster.highest_prime){
                for(int i = myMaster.highest_prime+1; i< nb_test; i++){ 
                    //on envoie chaque valeur entre le max+1 et le nombre à tester
                    //on envoie cette valeur au(x) worker(s) une par une 
                    im_Writing(myMaster.ecriture, i);

                    //attente de la réponse des workers
                    result = im_Reading(myMaster.lecture);

                    if(result == IS_PRIME){
                        myMaster.highest_prime = result;
                        myMaster.howmany_prime++;
                    }
                }
            }
            
            //on envoie le nombre à tester au worker
            im_Writing(myMaster.ecriture, nb_test);
            //on lit la réponse du worker 
            result = im_Reading(myMaster.lecture);
            
            //vérification du plus haut nombre premier calculé
            //pour mettre à jour les données du master
            if(result == IS_PRIME){
                //une fois la réponse obtenue 1 ou -1 on l'a transmet au client 
                ret = write(master_client, &result, sizeof(int));
                myassert(ret != -1, "Ecriture du résultat au client compromise");
                
                if(myMaster.highest_prime < nb_test){
                    myMaster.highest_prime = nb_test;
                    //incrémentation pour mettre à jour le nombre de premier calculé

                    myMaster.howmany_prime++;
                }
            }else if(result == NO_PRIME){
                ret = write(master_client, &result, sizeof(int));
                myassert(ret != -1, "Ecriture du résultat au client compromise");
            }

        }else if(command == ORDER_HOW_MANY_PRIME) {
                printf("\nOrdre HowMany Prime");
                //on envoie la donnée howmany_prime stockée dans master (structure)
                ret = write(master_client, &myMaster.howmany_prime, sizeof(int));
                myassert(ret == sizeof(int), "ecriture compromise");

            }else if(command == ORDER_HIGHEST_PRIME) {
                printf("\nOrdre Highest Prime");

                //on envoie la donnée highest_prime stockée dans master
                ret = write(master_client, &myMaster.highest_prime, sizeof(int));
                myassert(ret == sizeof(int), "ecriture compromise");
            }
        
        sleep(2);
        // - fermer les tubes nommés
        close(master_client);
        close(client_master);

        if(command == -1){
                printf("\n Le master va se fermer ...\n "); 
                break;
        }
    }
}

master_data init_MyMaster(master_data mymaster){
    int ret = pipe(mymaster.master_to_worker);
    myassert(ret != 1, "init pipe master_worker compromise");
    int ret1 = pipe(mymaster.worker_to_master);
    myassert(ret1 != 1, "init pipe worker_master compromise");
    
    mymaster.highest_prime = INIT_VALUE;
    mymaster.howmany_prime = INIT_VALUE;

    return mymaster;
}
/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
     if (argc != 1)
        usage(argv[0], NULL);

    //Initialisation des pipes de la struct master
    master_data myMaster = init_MyMaster(myMaster);

    // - création de la sémaphore mutex pour les clients
    int sema_mutex = semget(CLE_CLIENT, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(sema_mutex != -1, "le semaphore ne s'est pas creer");
    int init_sema = semctl(sema_mutex, 0, SETVAL, 1);
    myassert(init_sema != -1 , "Le semaphore s'est mal initialisé");

    // - création des tubes nommés
    int tube_mc = mkfifo(PIPE_MASTER_TO_CLIENT, 0644);
    myassert(tube_mc != -1, "problème au niveau du tube master vers client");
    int tube_cm = mkfifo(PIPE_CLIENT_TO_MASTER, 0644);
    myassert(tube_cm != 1, "problème au niveau du tube client vers master");
    
    // - création du premier worker
    int prime_origine = 2;

    pid_t ret_fork = fork();
    myassert(ret_fork != -1, "problème lors de la créatio du premier worker");

    if(ret_fork == 0){
        myMaster.ecriture = mode_write(myMaster.worker_to_master);
        myMaster.lecture = mode_read(myMaster.master_to_worker);

        worker_creation(prime_origine, myMaster.ecriture, myMaster.lecture);
    }

    //Gestion des lectures/écritures entre master/worker
    myMaster.ecriture = mode_write(myMaster.master_to_worker);
    myMaster.lecture = mode_read(myMaster.worker_to_master);
    
    
    //TEST obligatoire sinon bloquage sur 1 ??? 
    im_Writing(myMaster.ecriture, prime_origine);

    int test = im_Reading(myMaster.lecture);
    if(test == IS_PRIME){
        printf("\nworker reconnait %d comme nb premier\n",prime_origine);
        myMaster.highest_prime = prime_origine;
        myMaster.howmany_prime++;
    }

    // boucle infinie
    loop(myMaster);

    // destruction des tubes nommés, des sémaphores, ...
    
    int delete_sema = semctl(sema_mutex, -1, IPC_RMID);
    myassert(delete_sema != -1, "la semaphore mutex s'est mal détruite");

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