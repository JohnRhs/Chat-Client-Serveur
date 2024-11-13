#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "protocole.h"

#define MAX_CLIENTS 10
#define MAX_GROUPS 10
#define PSEUDO_SIZE 26

// Structure pour stocker les informations d'un client
typedef struct
{
	int		sockfd;
	struct	sockaddr_in addr;
	char	pseudo[PSEUDO_SIZE];
	int		online;
}			client_data;

// Structure pour stocker les informations d'un groupe
typedef struct
{
    char	group_id[PSEUDO_SIZE];
    int		members[MAX_CLIENTS];
    int		nb_members;
	int		max_members;
}			group_data;

client_data clients[MAX_CLIENTS]; // Tableau pour stocker les informations des clients
int nb_clients = 0; // Nombre de clients actuellement connectés

group_data groups[MAX_GROUPS]; // Tableau pour stocker les informations des groupes
int nb_groups = 0; // Nombre de groupes

void	init(void);
int		is_pseudo_valid(char box[]);
int		search_client_index(char pseudo[]);
int		search_group_index(char grp_id[]);
int		search_member_index(int client_index, int grp_index);
int		search_free_client_spot(void);
int		search_free_grp_spot(void);
int		search_free_member_spot(int grp_index);
void	make_client_list(char client_list[]);
void	make_groups_list(char groups_list[]);
void	make_members_list(char members_list[], int grp_index);
void	handle_name(int client_index, char buffer[]);
void	handle_priv(int client_index, char buffer[]);
void	handle_mess(int client_index, char buffer[]);
void	handle_cgrp(int client_index, char buffer[]);
void	handle_join(int client_index, char buffer[]);
void	handle_quit(int client_index, char buffer[]);
void	handle_list(int client_index, char buffer[]);
void	handle_grpl(int client_index, char buffer[]);
void	handle_memb(int client_index, char buffer[]);
void	handle_exit(int client_index, char buffer[]);
void	*handle_client(void *arg);

#endif