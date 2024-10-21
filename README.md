PROJET CHAT EN RESEAU TCP/IP : RAHARISON JOHN 22008899


- Le Projet implémente un Chat en réseau dans le langage C, il permet de connecter plusieurs clients au serveur, de leurs faire choisir un pseudo et discuter en privé, en public ou en groupe.



FONCTIONNALITE :
- Enregistrer son pseudo(NAME <pseudo>)
- Envoyer des messages à tout les clients(TOUS <message>) 
- Envoyer des messages privés(PRIV <nom> <message>)
- Créer Groupe (CGRP <taille> <nom>)
- Rejoindre Groupe (JOIN <groupe>)
- Quitter Groupe (QUIT <groupe>)
- Envoyer Messages au groupe (MESS <groupe> <message>)
- Voir tout les membres d'un groupe (MEMB <groupe>)
- Voir tout les clients du serveur (LIST)
- Voir Liste de tout les groupes (GRPL) 



COMPILATION : gcc serveur.c -o serveur
	      gcc client.c -o client

EXECUTION : Serveur -> ./serveur
            Client -> ./client P8 
NOTE : le serveur et le client se connectent automatiquement sur le localhost par défaut pour les tests.




CODE :

- Utilisation des Threads pour gérer la mutlipléxité de connexion avec les clients (après essais avec fork()).

- Reprise de toutes les fonctions réseaux utiliser pour le TP6(NETCAT) pour gérer les connexions Client/Serveur.

- Usage de Structures pour les clients et les groupes.

- Usage de Poll() côté client pour afficher sur STDOUT depuis la socket connecté au serveur, et envoyer depuis STDIN à cette même socket (pour traiter commande, messages, pseudo etc..).

- jeu avec send, write pour afficher traces dans le serveur ou aux clients.

- Pas réussi l'implémentation correct de PING/PONG.
