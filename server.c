#include "server.h"

void	init(void){
	int i,j;
	for (i = 0; i < MAX_GROUPS; i++){
		for (j = 0; j < MAX_CLIENTS; j++){
			groups[i].members[j] = -1;
		}
	}
}

int	is_pseudo_valid(char box[]){
	int i = 0;
	while (box[i] != '\0'){
		if (box[i] == ' ' || box[i] == '\t')
			return 0;
		i++;
	}
	if (i < 1 || i > 25)
		return 0;
	return 1;
}

int	search_client_index(char pseudo[]){
	for (int i = 0; i < MAX_CLIENTS; i++){
		if (strcmp(clients[i].pseudo, pseudo) == 0)
			return i;
	}
	return -1;
}

int	search_group_index(char grp_id[]){
	for (int i = 0; i < MAX_GROUPS; i++){
		if (strcmp(groups[i].group_id, grp_id) == 0)
			return i;
	}
	return -1;
}

int	search_member_index(int client_index, int grp_index){
	for (int i = 0; i < groups[grp_index].max_members; i++){
		if (groups[grp_index].members[i] == client_index)
			return i;
	}
	return -1;
}

int	search_free_client_spot(void){
	for (int i = 0; i < MAX_CLIENTS; i++){
		if (clients[i].online == 0)
			return i;
	}
	return -1;
}

int	search_free_grp_spot(void){
	for (int i = 0; i < MAX_GROUPS; i++)
	{
		if (groups[i].nb_members == 0)
			return i;
	}
	return -1;
}

int	search_free_member_spot(int grp_index){
	for (int i = 0; i < groups[grp_index].max_members; i++){
		if (groups[grp_index].members[i] == -1)
			return i;
	}
	return -1;
}

void make_client_list(char client_list[]){
	int i,j,k;
	j = 0;
	for (i = 0; i < MAX_CLIENTS; i++){
		if (clients[i].online == 1){
			client_list[j] = '\n';
			j++;
			k = 0;
			while (clients[i].pseudo[k] != '\0'){
				client_list[j] = clients[i].pseudo[k];
				j++;
				k++;
			}
		}
	}
	client_list[j] = '\0';
}

void make_groups_list(char groups_list[]){
	int i,j,k;
	j = 0;
	for (i = 0; i < MAX_GROUPS; i++){
		if (groups[i].nb_members != 0){
			groups_list[j] = '\n';
			j++;
			k = 0;
			while (groups[i].group_id[k] != '\0'){
				groups_list[j] = groups[i].group_id[k];
				j++;
				k++;
			}
		}
	}
	groups_list[j] = '\0';
}

void make_members_list(char members_list[], int grp_index){
	int i,j,k;
	j = 0;
	for (i = 0; i < groups[grp_index].max_members; i++){
		if (groups[grp_index].members[i] != -1){
			members_list[j] = '\n';
			j++;
			k = 0;
			while (clients[groups[grp_index].members[i]].pseudo[k] != '\0'){
				members_list[j] = clients[groups[grp_index].members[i]].pseudo[k];
				j++;
				k++;
			}
		}
	}
	members_list[j] = '\0';
}

void handle_name(int client_index, char buffer[]){
	char pseudo[BUFFER_SIZE];
	int	size;
	// Parsing du buffer et gestion d'erreur
	parse_1box(buffer, pseudo);
	if (strncmp(buffer, "NAME\0", TAG_SIZE) != 0){
		send(clients[client_index].sockfd, "NOPE 10\n\0", strlen("NOPE 10\n\0"), 0);
		return ;
	}
	if (!(is_pseudo_valid(pseudo))){
		send(clients[client_index].sockfd, "NOPE 12\n\0", strlen("NOPE 12\n\0"), 0);
		return ;
	}
	if (search_client_index(pseudo) != -1){
		send(clients[client_index].sockfd, "NOPE 13\n\0", strlen("NOPE 13\n\0"), 0);
		return ;
	}
	// Ajout du pseudo au tableau des clients et envoi de HELO <pseudo> au client
	ft_strcpy(clients[client_index].pseudo, pseudo);
	clients[client_index].online = 1;
	size = make_request_1box(buffer, clients[client_index].pseudo, "HELO\0");
	send(clients[client_index].sockfd, buffer, size, 0);
}

void	handle_priv(int client_index, char buffer[]){
	char pseudo[BUFFER_SIZE];
	char message[BUFFER_SIZE];
	int	dest,size;

	// Parsing du buffer et gestion d'erreur
	parse_2box(buffer, pseudo, message);
	dest = search_client_index(pseudo);
	if (dest == -1){
		send(clients[client_index].sockfd, "NOPE 14\n\0", strlen("NOPE 14\n\0"), 0);
		return ;
	}

	// Envoi de PRIV <pseudo> <message> au destinataire
	size = make_request_2box(buffer, clients[client_index].pseudo, message, "PRIV\0");
	send(clients[dest].sockfd, buffer, size, 0);
	send(clients[client_index].sockfd, buffer, size, 0);
}

void handle_mess(int client_index, char buffer[])
{
	char grp_id[BUFFER_SIZE];
	char message[BUFFER_SIZE];
	int	grp_index, size;

	// Parsing du buffer et gestion d'erreur
	parse_2box(buffer, grp_id, message);
	grp_index = search_group_index(grp_id);
	if (grp_index == -1){
		send(clients[client_index].sockfd, "NOPE 14\n\0", strlen("NOPE 14\n\0"), 0);
		return ;
	}

	// Envoi de MESS <groupe> <pseudo> <message> dans le groupe
	size = make_request_3box(buffer, groups[grp_index].group_id, clients[client_index].pseudo, message, "MESS\0");
	for (int i = 0; i < groups[grp_index].max_members; i++){
		if (groups[grp_index].members[i] != -1){
			send(clients[groups[grp_index].members[i]].sockfd, buffer, size, 0);
		}
	}
}

void handle_cgrp(int client_index, char buffer[])
{
	char nombre[8];
	char grp_id[BUFFER_SIZE];
	char members_list[BUFFER_SIZE];
	int	grp_index, size, max_members;

	// Parsing du buffer et gestion d'erreur
	parse_2box(buffer, nombre, grp_id);
	max_members = atoi(nombre);
	if (max_members < 1 || max_members > MAX_CLIENTS){
		send(clients[client_index].sockfd, "NOPE 17\n\0", strlen("NOPE 17\n\0"), 0);
		return ;
	}
	if (is_pseudo_valid(grp_id) == 0){
		send(clients[client_index].sockfd, "NOPE 12\n\0", strlen("NOPE 12\n\0"), 0);
		return ;
	}
	if (search_group_index(grp_id) != -1){
		send(clients[client_index].sockfd, "NOPE 13\n\0", strlen("NOPE 13\n\0"), 0);
		return ;
	}

	// Update du tableau des groupes
	grp_index = search_free_grp_spot();
	ft_strcpy(groups[grp_index].group_id, grp_id);
	groups[grp_index].members[0] = client_index;
	groups[grp_index].nb_members = 1;
	groups[grp_index].max_members =  max_members;
	nb_groups++;

	// Envoi de INTO <groupe> <N> + liste des pseudos des membres du groupe au client
	ft_itoa(nombre, groups[grp_index].nb_members);
	make_members_list(members_list, grp_index);
	size = make_request_3box(buffer, groups[grp_index].group_id, nombre, members_list, "INTO\0");
	send(clients[client_index].sockfd, buffer, size, 0);
}

void	handle_join(int client_index, char buffer[]){
	char nombre[8];
	char grp_id[BUFFER_SIZE];
	char members_list[BUFFER_SIZE];
	int	grp_index, member_index, size;

	// Parsing du buffer et gestion d'erreur
	parse_1box(buffer, grp_id);
	grp_index = search_group_index(grp_id);
	if (grp_index == -1){
		send(clients[client_index].sockfd, "NOPE 14\n\0", strlen("NOPE 14\n\0"), 0);
		return ;
	}
	if (groups[grp_index].nb_members == groups[grp_index].max_members){
		send(clients[client_index].sockfd, "NOPE 16\n\0", strlen("NOPE 16\n\0"), 0);
		return ;
	}

	// Update du tableau des groupes
	member_index = search_free_member_spot(grp_index);
	groups[grp_index].members[member_index] = client_index;
	groups[grp_index].nb_members++;

	// Envoi de INTO <groupe> <N> + liste des pseudos des membres du groupe au client
	ft_itoa(nombre, groups[grp_index].nb_members);
	make_members_list(members_list, grp_index);
	size = make_request_3box(buffer, groups[grp_index].group_id, nombre, members_list, "INTO\0");
	send(clients[client_index].sockfd, buffer, size, 0);

	// Envoi de ANEW <pseudo> dans le groupe
	size = make_request_1box(buffer, clients[client_index].pseudo, "ANEW\0");
	for (int i = 0; i < groups[grp_index].max_members; i++){
		if (groups[grp_index].members[i] != -1 && groups[grp_index].members[i] != client_index){
			send(clients[groups[grp_index].members[i]].sockfd, buffer, size, 0);
		}
	}
}

void	handle_quit(int client_index, char buffer[]){
	char grp_id[BUFFER_SIZE];
	int	grp_index, member_index, size;

	// Parsing du buffer et gestion d'erreur
	parse_1box(buffer, grp_id);
	grp_index = search_group_index(grp_id);
	if (grp_index == -1){
		send(clients[client_index].sockfd, "NOPE 14\n\0", strlen("NOPE 14\n\0"), 0);
		return ;
	}
	member_index = search_member_index(client_index, grp_index);
	if (member_index == -1){
		send(clients[client_index].sockfd, "NOPE 18\n\0", strlen("NOPE 18\n\0"), 0);
		return ;
	}

	// Update du tableau des groupe et envoi de LEFT au client
	groups[grp_index].members[member_index] = -1;
	groups[grp_index].nb_members--;
	send(clients[client_index].sockfd, "LEFT\n\0", 6, 0);

	// Si le groupe n'est pas vide, envoi de QUIT <pseudo> dans le groupe
	if (groups[grp_index].nb_members > 0){
		size = make_request_1box(buffer, clients[client_index].pseudo, "QUIT\0");
		for (int i = 0; i < groups[grp_index].max_members; i++){
			if (groups[grp_index].members[i] != -1){
				send(clients[groups[grp_index].members[i]].sockfd, buffer, size, 0);
			}
		}
	}

	// Sinon suppression du groupe
	else{
		memset(groups[grp_index].group_id, 0, PSEUDO_SIZE);
		groups[grp_index].max_members = 0;
		nb_groups--;
	}
}

void handle_list(int client_index, char buffer[])
{
	char nombre[8];
	char client_list[BUFFER_SIZE];
	int	size;

	// Envoi de la liste des clients connectés au client
	ft_itoa(nombre, nb_clients);
	make_client_list(client_list);
	size = make_request_2box(buffer, nombre, client_list, "LIST\0");
	send(clients[client_index].sockfd, buffer, size, 0);
}

void handle_grpl(int client_index, char buffer[]){
	char nombre[8];
	char grp_list[BUFFER_SIZE];
	int	size;

	// Envoi de GRPL <N> + la liste des membres du groupe au client
	ft_itoa(nombre, nb_groups);
	make_groups_list(grp_list);
	size = make_request_2box(buffer, nombre, grp_list, "GRPL\0");
	send(clients[client_index].sockfd, buffer, size, 0);
}

void	handle_memb(int client_index, char buffer[]){
	char nombre[8];
	char grp_id[BUFFER_SIZE];
	char members_list[BUFFER_SIZE];
	int	grp_index, size;

	// Parsing du buffer et gestion d'erreur
	parse_1box(buffer, grp_id);
	grp_index = search_group_index(grp_id);
	if (grp_index == -1){
		send(clients[client_index].sockfd, "NOPE 14\n\0", strlen("NOPE 14\n\0"), 0);
		return ;
	}

	// Envoi de INTO <N> + la liste des membres du groupe au client
	make_members_list(members_list, grp_index);
	ft_itoa(nombre, groups[grp_index].nb_members);
	size = make_request_3box(buffer, groups[grp_index].group_id, nombre, members_list, "INTO\0");
	send(clients[client_index].sockfd, buffer, size, 0);
}

void handle_exit(int client_index, char buffer[])
{
	int	i,j,size;

	printf("Client %d est déconnecté.\n", client_index);

	// Enlever le client des groupes
	for(i = 0; i < MAX_GROUPS; i++){
		for (j = 0; j < groups[i].max_members; j++){
			if (groups[i].members[j] == client_index){
				groups[i].members[j] = -1;
				groups[i].nb_members--;
				if (groups[i].nb_members == 0){
					memset(groups[i].group_id, 0, PSEUDO_SIZE);
					groups[i].max_members = 0;
					nb_groups--;
				}
			}
		}
	}

	// Envoi de de EXIT <pseudo> à tout le monde
	size = make_request_1box(buffer, clients[client_index].pseudo, "EXIT\0");
	for (i = 0; i < MAX_CLIENTS; i++){
		if (clients[i].online == 1){
			send(clients[i].sockfd, buffer, size, 0);
		}
	}

	// Update du tableau des clients
	clients[client_index].sockfd = 0;
	memset(&clients[client_index].addr, 0, sizeof(clients[client_index].addr));
	memset(clients[client_index].pseudo, 0, PSEUDO_SIZE);
	clients[client_index].online = 0;
	nb_clients--;
}

// Fonction pour gérer la communication avec un client
void *handle_client(void *arg)
{
	int	client_index = *((int *)arg);
	char buffer[BUFFER_SIZE];
	int	n;

	printf("Client %d est connecté.\n", client_index);

	while ((n = recv(clients[client_index].sockfd, buffer, sizeof(buffer), 0)) > 0){
		printf("hoho");
		if (clients[client_index].pseudo[0] == '\0'){
			handle_name(client_index, buffer);
			continue ;
		}
		if (strncmp(buffer, "NAME\0", TAG_SIZE) == 0){
			send(clients[client_index].sockfd, "NOPE 11\n\0", strlen("NOPE 11\n\0"), 0);
		}
			// Envoi de message prive
		else if (strncmp(buffer, "PRIV\0", TAG_SIZE) == 0){
			handle_priv(client_index, buffer);
		}
			// Envoi de message de groupe
		else if (strncmp(buffer, "MESS\0", TAG_SIZE) == 0){
			handle_mess(client_index, buffer);
		}
			// Demande de la liste des clients connectés
		else if (strncmp(buffer, "LIST\0", TAG_SIZE) == 0){
			handle_list(client_index, buffer);
		}
			// Création d'un groupe
		else if (strncmp(buffer, "CGRP\0", TAG_SIZE) == 0){
			handle_cgrp(client_index, buffer);
		}
			// Rejoignement d'un groupe
		else if (strncmp(buffer, "JOIN\0", TAG_SIZE) == 0){
			handle_join(client_index, buffer);
		}
			// Dérejoignement d'un groupe
		else if (strncmp(buffer, "QUIT\0", TAG_SIZE) == 0){
			handle_quit(client_index, buffer);
		}
			// Demande de la liste des groupes
		else if (strncmp(buffer, "GRPL\0", TAG_SIZE) == 0){
			handle_grpl(client_index, buffer);
		}
			// Demande de la liste des membres d'un groupe
		else if (strncmp(buffer, "MEMB\0", TAG_SIZE) == 0){
			handle_memb(client_index, buffer);
		}
			// Déconnexion client
		else if (strncmp(buffer, "EXIT\0", TAG_SIZE) == 0){
			handle_exit(client_index, buffer);
			break;
		}
	}


	// Fermer la connexion avec le client
	close(clients[client_index].sockfd);
	free(arg);
	pthread_exit(NULL);
}

int	main(int argc, char *argv[]){
	(void)argv;
	int sockfd, clientfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addrlen = sizeof(client_addr);
	pthread_t tid[MAX_CLIENTS];
	int port = 8080;

	if (argc != 2){
		fprintf(stderr, "Usage : ./serveur <P8>\n");
		exit(EXIT_FAILURE); 
	}

	init();

	// Création du socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Initialisation de l'adresse du serveur
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	// Liaison du socket
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// Ecoute sur le socket
	if (listen(sockfd, SOMAXCONN) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1){
		// Accepter une connexion entrante
		clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
		if (clientfd < 0){
			perror("accept");
			continue ;
		}

		int *arg = malloc(sizeof(*arg));
		if (arg == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		// Ajouter les informations du client au tableau clients
		*arg = search_free_client_spot();
		clients[*arg].sockfd = clientfd;
		memcpy(&clients[*arg].addr, &client_addr, sizeof(client_addr));
		memset(clients[*arg].pseudo, 0, PSEUDO_SIZE);
		clients[*arg].online = 0;
		nb_clients++;

		// Créer un thread pour gérer la communication avec le client
		if (pthread_create(&tid[*arg], NULL, handle_client, arg) != 0){
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
