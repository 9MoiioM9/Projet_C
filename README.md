# Projet_C

## Information sur le Projet 

Ce projet met en place une système de communication entre processus. Composé d'un client, un master et des workers. 
Le but est de faire une communication entre les processus, le client envoie des requêtes au master via des mots simples. Le master interprète la commande reçu, si il y a des calculs à faire alors les workers rentrent en scène. 

### Commandes Possibles

* ORDER_STOP **(Permet l'arrêt des calculs et la fermetures de tous les workers)**
* ORDER_COMPUTE_PRIME **(Donne un numéro et vérifie si celui-ci est premier ou non)**
* ORDER_HOW_MANY_PRIME **(Donne le nombre de nombre premier calculer au moment de la commande)**
* ORDER_HIGHEST_PRIME  **(Renvoie le nombre premier le plus élevé calculé)**

###
TODO Faire explication pour le fonctionnement

