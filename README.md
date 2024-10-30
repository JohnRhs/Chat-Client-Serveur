Projet Chat en Réseau TCP/IP

Auteur: Raharison John

Ce projet implémente un chat en réseau en utilisant le langage C, permettant à plusieurs clients de se connecter à un serveur. Les utilisateurs peuvent choisir un pseudo et discuter de manière publique, privée ou en groupe.

Fonctionnalités

Le projet inclut les fonctionnalités suivantes :

	•	Enregistrer un pseudo : NAME
	•	Envoyer un message à tous les clients : TOUS
	•	Envoyer un message privé : PRIV
	•	Créer un groupe : CGRP
	•	Rejoindre un groupe : JOIN
	•	Quitter un groupe : QUIT
	•	Envoyer un message à un groupe : MESS
	•	Voir les membres d’un groupe : MEMB
	•	Voir tous les clients connectés au serveur : LIST
	•	Voir la liste des groupes : GRPL

Compilation

Pour compiler le serveur et le client, exécutez les commandes suivantes :

	•	gcc serveur.c -o serveur
	•	gcc client.c -o client

Exécution

L’exécution se fait comme suit :

Serveur :

	•	./serveur 

Client :


	•	./client P8




Par défaut, le serveur et le client se connectent automatiquement sur localhost pour les tests.

Détails techniques

	•	Threads : Utilisation des threads pour gérer la multiplicité des connexions clients après des essais avec fork().
	•	Fonctions réseau : Reprise des fonctions réseau utilisées dans le TP6 (NETCAT) pour la gestion des connexions client/serveur.
	•	Structures : Utilisation de structures pour gérer les clients et les groupes.
	•	Poll() : Côté client, poll() est utilisé pour :
	•	Afficher sur STDOUT les messages reçus depuis la socket connectée au serveur.
	•	Envoyer les commandes et messages depuis STDIN vers cette même socket (pseudos, messages, etc.).
	•	send() et write() : Utilisés pour afficher les traces sur le serveur et chez les clients.
	•	PING/PONG : L’implémentation de cette fonctionnalité n’a pas pu être correctement finalisée.

Ce README permet d’avoir une présentation claire et précise de votre projet, tout en respectant les bonnes pratiques pour un affichage sur GitHub.
