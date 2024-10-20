#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_EVENTS 2
#define MAX_LETTER 26
#define MAX_CLT 10
#define MAX_GRP 5

typedef struct {
  int sockfd;
  struct sockaddr_in addr;
  char nom[MAX_LETTER];
} clt;

clt clients[MAX_CLT];
int nb_clients = 0;

void init_clt(void) {
  for (int i = 0; i < MAX_CLT; i++) {
    clients[i].nom[0] = '\0';
  }
}

typedef struct {
  clt membres[1024];
  int nb_memb;
  char nom_grp[MAX_LETTER];
  int taille;
} grp;

grp groupes[MAX_GRP];
int nb_groupes = 0;

void init_grp(void) {
  for (int i = 0; i < MAX_GRP; i++) {
    groupes[i].nb_memb = 0;
    groupes[i].nom_grp[0] = '\0';
    groupes[i].taille = 0;
  }
}

int valid_nom(char name[]){
  int i = 0;
  while (name[i] != '\0'){
    if (name[i] == ' ')
      return 0;
    i++;
  }
  if (i < 1 || i > 25)
    return 0;
  return 1;
}

// --------- SERVEUR ----------- //

void groupe(char name[], int client_index, int max_memb) {
  name[strlen(name) - 1] = '\0';

  if (nb_groupes >= MAX_GRP) {
    send(clients[client_index].sockfd, "NOPE 15\n\n\0", 10, 0);
    return;
  }

  if (!valid_nom(name)) {
    send(clients[client_index].sockfd, "NOPE 12\n\n\0", 10, 0);
    return;
  }

  for (int i = 0; i < nb_groupes; i++) {
    if (strcmp(groupes[i].nom_grp, name) == 0) {
      send(clients[client_index].sockfd, "NOPE 13\n\n\0", 10, 0);
      return;
    }
  }

  // Cration du groupe
  strcpy(groupes[nb_groupes].nom_grp, name);
  memcpy(&groupes[nb_groupes].membres[0], &clients[client_index], sizeof(clt));
  groupes[nb_groupes].nb_memb = 1;
  groupes[nb_groupes].taille = max_memb;
  nb_groupes++;

  char cgrp[50];
  sprintf(cgrp, "Vous avez créé un nouveau groupe : %s\n", name);
  char memb[1024];
  sprintf(memb, "%s\n", groupes[nb_groupes - 1].membres[0].nom);
  send(clients[client_index].sockfd, cgrp, strlen(cgrp), 0);
  printf("INTO %s %d\n", name, nb_groupes);
  printf("%s\n", memb);
  send(clients[client_index].sockfd, memb, strlen(memb), 0);
  send(clients[client_index].sockfd, "\n", 1, 0);
}


void message(void* buffer, char cmd[], char name[], char mess[]){
  int i = 0, j = 0;
  const char* ptr;
  ptr = buffer;
  
  while (*(ptr+i) != '\0' && i < 4){
    cmd[i] = *(ptr+i);
    i++;
  }
  cmd[i] = '\0';
  
  while (*(ptr+i) != '\0' && *(ptr+i) == ' ' && *(ptr+i) != '\n')
    i++;
  while (*(ptr+i) != '\0' && *(ptr+i) != ' ' && *(ptr+i) != '\n'){
    name[j] = *(ptr+i);
    i++;
    j++;
  }
  name[j] = '\0';

  j = 0;
  while (*(ptr+i) != '\0' && *(ptr+i) == ' ' && *(ptr+i) != '\n')
    i++;
  while (*(ptr+i) != '\0'){
    mess[j] = *(ptr+i);
    i++;
    j++;
  }
  mess[j] = '\0';
}

int valid_clt(char name[]){
  for (int j = 0; j < nb_clients; j++){
    if (strcmp((char *)clients[j].nom, (char *)name) == 0){
      return 1;
    }
  }
  return 0;
}

void *connexion_client(void *arg){
  int client_index = *((int *)arg);
  char buffer[1024];
  char cmd[5];
  char name[1024];
  char mess[1024];
  char helo[50] = "HELO \0";
  int n, i;


  while ((n = recv(clients[client_index].sockfd, buffer, sizeof(buffer), 0)) > 0){  
    // CMD, NOM, MESS
    message(buffer, cmd, name, mess);

    // NAME
    if (clients[client_index].nom[0] == '\0'){
      if (strcmp(cmd, "NAME\0") != 0){
        send(clients[client_index].sockfd, "NOPE 10\n\n\0", 10, 0);
      }
      else if (!(valid_nom(name))){
        send(clients[client_index].sockfd, "NOPE 12\n\n\0", 10, 0);
      }
      else if (valid_clt(name)){
        send(clients[client_index].sockfd, "NOPE 13\n\n\0", 10, 0);
      }
      else {
        strcpy(clients[client_index].nom, name);
        strcpy(helo + 5, name);
        helo[strlen(helo)] = '\n';

        char bienv[1024];
        sprintf(bienv, "Bienvenue! %s\n\n", name);
        send(clients[client_index].sockfd, bienv, 50, 0);
        printf("%s\n", helo);
      }
    }
    // Si le client retape NAME 
    else{
      if (strcmp(cmd, "NAME\0") == 0){
        send(clients[client_index].sockfd, "NOPE 11\n\n\0", 10, 0);
      }
      // CRGP //
      else if (strcmp(cmd, "CGRP\0") == 0) {
        int max_memb = atoi(name);
        char group_name[MAX_LETTER];
        strcpy(group_name, mess);
        groupe(group_name, client_index, max_memb);
      }
      
      // PRIV //
      else if (strcmp(cmd, "PRIV\0") == 0) {
        char priv[1024];
        sprintf(priv, "%s(MESSAGE PRIVE) : %s", clients[client_index].nom, mess);
    
        for (int j = 0; j < nb_clients; j++) {
          if (strcmp(clients[j].nom, name) == 0) {
            send(clients[j].sockfd, priv, strlen(priv), 0);
          }
        }
      }

      else if (strcmp(cmd, "QUIT") == 0){
        char quit[1024];
        char left[1024];
        for (int i = 0; i < nb_groupes; i++) {
          if (strcmp(groupes[i].nom_grp, name) == 0) {
            for (int j = 0; j < groupes[i].nb_memb; j++) {
              if (strcmp(groupes[i].membres[j].nom, clients[client_index].nom) == 0) {
                for (int k = j; k < groupes[i].nb_memb - 1; k++) {
                  memcpy(&groupes[i].membres[k], &groupes[i].membres[k + 1], sizeof(clt));
                }
                groupes[i].nb_memb--;
                sprintf(left, "Groupe <%s> quitté\n\n", name);
                sprintf(quit, "%s a quitté le groupe : %s\n\n", clients[client_index].nom, name);
                write(STDOUT_FILENO, "LEFT ", 5);
                write(STDOUT_FILENO, name, strlen(name));
                write(STDOUT_FILENO, "\n\n", 2);
                send(clients[client_index].sockfd, left, strlen(left), 0);
                if (groupes[i].nb_memb == 0) {
                  for (int k = i; k < nb_groupes - 1; k++) {
                    memcpy(&groupes[k], &groupes[k + 1], sizeof(grp));
                  }
                  nb_groupes--;
                }
                else{
                  for (int j = 0; j < groupes[i].nb_memb; j++) {
                    if (strcmp(groupes[i].membres[j].nom, clients[client_index].nom) != 0){
                      send(groupes[i].membres[j].sockfd, quit, strlen(quit), 0);
                    }
                  }
                }
              }
            }
          }else{
            send(clients[client_index].sockfd, "NOPE 14\n\n\0", 10, 0);
            break;
          }
        }
      }

      // JOIN //
      else if (strcmp(cmd, "JOIN") == 0) {
        int n = 0;
        for (int i = 0; i < nb_groupes; i++) {
          if (strcmp(groupes[i].nom_grp, name) == 0) {
            n = 1; 
            if (groupes[i].nb_memb >= groupes[i].taille) {
              send(clients[client_index].sockfd, "NOPE 16\n\n\0", 10, 0);
              break;
            }
            for (int j = 0; j < groupes[i].nb_memb; j++) {
              if (strcmp(groupes[i].membres[j].nom, clients[client_index].nom) == 0) {
                send(clients[client_index].sockfd, "NOPE 11\n\n\0", 10, 0);
                break;
              }else{
                memcpy(&groupes[i].membres[groupes[i].nb_memb], &clients[client_index], sizeof(clt));
                groupes[i].nb_memb++;
                char join[1024];
                sprintf(join, "Groupe <%s> rejoint\n\n", name);
                send(clients[client_index].sockfd, join, strlen(join), 0);
                write(STDOUT_FILENO, "INTO ", 5);
                write(STDOUT_FILENO, name, strlen(name));
                write(STDOUT_FILENO, "\n", 1);
                write(STDOUT_FILENO, clients[client_index].nom, strlen(clients[client_index].nom));
                write(STDOUT_FILENO, "\n\n", 2);
                for(int k = 0; k < groupes[i].nb_memb; k++){
                  send(groupes[k].membres[k].sockfd, "ANEW membre : ", 14, 0);
                  send(groupes[k].membres[k].sockfd, clients[client_index].nom, strlen(clients[client_index].nom), 0);
                  send(groupes[k].membres[k].sockfd, "\n\n", 2, 0);
                }
                break;
              } 
            }
          }
        }
        if (!n) {
          send(clients[client_index].sockfd, "NOPE 14\n\n\0", 10, 0);
        }
      }

      // GRPL //
      else if (strcmp(cmd, "GRPL\0") == 0){
        char grpl[1024]; 
        sprintf(grpl, "Groupes(%d) : ", nb_groupes);
        send(clients[client_index].sockfd, grpl, strlen(grpl), 0);
        write(STDOUT_FILENO, "GRPL ", 5);
        // Envoyer chaque nom de client séparément
        for (int j = 0; j < nb_groupes; j++){
          send(clients[client_index].sockfd, groupes[j].nom_grp, strlen(groupes[j].nom_grp), 0);
          write(STDOUT_FILENO, groupes[j].nom_grp, strlen(groupes[j].nom_grp));
          send(clients[client_index].sockfd, " ", 1, 0);
        }
        send(clients[client_index].sockfd, "\n\n", 2, 0);
        write(STDOUT_FILENO, "\n\n", 2);
      }
      
      // LIST //
      else if (strcmp(cmd, "LIST\0") == 0){
        char list[1024]; 
        sprintf(list, "Clients connectés(%d) : ", nb_clients);
        send(clients[client_index].sockfd, list, strlen(list), 0);
        write(STDOUT_FILENO, "LIST ", 5);
        // Envoyer chaque nom de client séparément
        for (int j = 0; j < nb_clients; j++){  
            send(clients[client_index].sockfd, clients[j].nom, strlen(clients[j].nom), 0);
            write(STDOUT_FILENO, clients[j].nom, strlen(clients[j].nom));
            write(STDOUT_FILENO, " ", 1);
            send(clients[client_index].sockfd, " ", 1, 0);
        }
        send(clients[client_index].sockfd, "\n\n", 2, 0);
        write(STDOUT_FILENO, "\n\n", 2);
      }

      // MESS (GROUPE)
      else if (strcmp(cmd, "MESS") == 0) {
        char message[1024];
        char out[1024];
        sprintf(out, "MESS %s %s \n\n", clients[client_index].nom, name);
        int n = 0; // Flag to indicate if the client is a member of the group

        // Check if the client is a member of the group
        for (int i = 0; i < nb_groupes; i++) {
          if (strcmp(groupes[i].nom_grp, name) == 0) {
            for (int j = 0; j < groupes[i].nb_memb; j++) {
              if (strcmp(groupes[i].membres[j].nom, clients[client_index].nom) == 0) {
                n = 1;
                break;
              }
            }
            break;
          }
        }

        if (n) {
          sprintf(message, "%s (MESSAGE GROUPE -> %s) : %s\n", clients[client_index].nom, name, mess);
          for (int i = 0; i < nb_groupes; i++) {
            if (strcmp(groupes[i].nom_grp, name) == 0) {
              for (int j = 0; j < groupes[i].nb_memb; j++) {
                send(groupes[i].membres[j].sockfd, message, strlen(message), 0);
                write(STDOUT_FILENO, out, strlen(out));
              }
              break;
            }
          }
        } else {
          send(clients[client_index].sockfd, "NOPE 18\n\n", 9, 0);
        }
      }
      
      // TOUS // (ENVOI DE MESSAGE A TOUT LE SEVEUR)
      else if (strcmp(cmd, "TOUS\0") == 0) {
        char tous[1024];
        sprintf(tous, "%s(MESSAGE PUBLIC) : %s\n", clients[client_index].nom, name);
        for (int i = 0; i < nb_clients; i++) {
          if (i != client_index) {
            send(clients[i].sockfd, tous, strlen(tous), 0);
          }
        }
      }
      // MEMB //
      else if(strcmp(cmd, "MEMB") == 0) {
        char memb[1024]; 
        for (int i = 0; i < nb_groupes; i++) {
          if (strcmp(groupes[i].nom_grp, name) == 0) {
            sprintf(memb, "Membres du Groupe %s (%d) : ", groupes[i].nom_grp, groupes[i].nb_memb);
            send(clients[client_index].sockfd, memb, strlen(memb), 0);
            write(STDOUT_FILENO, "MEMB ", 5);
            for (int j = 0; j < groupes[i].nb_memb; j++) {
              send(clients[client_index].sockfd, groupes[i].membres[j].nom, strlen(groupes[i].membres[j].nom), 0);
              write(STDOUT_FILENO, groupes[i].membres[j].nom, strlen(groupes[i].membres[j].nom));
              write(STDOUT_FILENO, " ", 1);
              send(clients[client_index].sockfd, " ", 1, 0);
            }
          }else{
            send(clients[client_index].sockfd, "NOPE 14\n\n\0", 10, 0);
          }
        }
        // Envoyer chaque nom de client séparément
        send(clients[client_index].sockfd, "\n\n", 2, 0);
        write(STDOUT_FILENO, "\n\n", 2);
      }


      // EXIT //
      else if(strcmp(cmd, "EXIT\0") == 0){
        char exit[100];
        char quit[100];

        sprintf(exit, "A bientot à -- P8 CHATROOM %s !", clients[client_index].nom);
        sprintf(quit, "%s nous a quitté\n\n", clients[client_index].nom);
        send(clients[client_index].sockfd, exit, strlen(exit), 0);
        if (clients[client_index].nom[0] != '\0') {
          printf("EXIT %s\n\n", clients[client_index].nom);
        } else {
          printf("EXIT Anonyme \n\n");
        }

        for (int i = 0; i < nb_clients; i++) {
          if (i != client_index) {
            send(clients[i].sockfd, quit, strlen(quit), 0);
          }
        }
        
        close(clients[client_index].sockfd);
        clients[client_index].nom[0] = '\0';
        memset(&clients[client_index].addr, 0, sizeof(clients[client_index].addr));
        memset(clients[client_index].nom, 0, MAX_LETTER);
        for (int i = client_index; i < nb_clients - 1; ++i) {
          clients[i] = clients[i + 1];
        }
        nb_clients--;
        // Supprimer le client des groupes
        for(int i = 0; i < MAX_GRP; i++){
          for (int j = 0; j < groupes[i].taille; j++){
            for (int k = client_index; k < groupes[i].nb_memb - 1; ++k){
              groupes[i].membres[k] = groupes[i].membres[k + 1];
            }
            groupes[i].nb_memb--;
            break; // Sortir de la boucle interne
          }
        }
      }
    }
  }
  if (clients[client_index].nom[0] != '\0') {
    printf("EXIT %s\n\n", clients[client_index].nom);
  }
  else {
    printf("EXIT Anonyme \n\n");
  }
  close(clients[client_index].sockfd);
  for (int i = client_index; i < nb_clients - 1; ++i) {
    clients[i] = clients[i + 1];
  }
  nb_clients--;
  free(arg);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(stderr, "Usage: %s\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  int sockfd, clientfd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addrlen = sizeof(client_addr);
  pthread_t tid[MAX_CLT];
  //Variables d'affichage
  struct addrinfo hints, *res;
  char ip[INET6_ADDRSTRLEN];
  int port;
  void *addr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi("5678"));

  init_clt();
  init_grp();

  if (getaddrinfo(NULL, "5678", &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    close(sockfd);
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, 10) < 0) {
    perror("listen");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // AFFICHAGE D'ECOUTE
  addr = (res->ai_family == AF_INET)
      ? (void *)&(((struct sockaddr_in *)res->ai_addr)->sin_addr)
      : (void *)&(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr);
  inet_ntop(res->ai_family, addr, ip, sizeof(ip));
  port = (res->ai_family == AF_INET)
      ? ((struct sockaddr_in *)res->ai_addr)->sin_port
          : ((struct sockaddr_in6 *)res->ai_addr)->sin6_port;
  printf("-- P8 CHATROOM -- sur le port -> %d (par défaut pour les tests)\n\n", ntohs(port));

  while (1) {
    if ((clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) {
      perror("accept");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    printf("Connexion établie avec un client.\n");

    // Ajouter les informations du client à la struct clt(clients)
    clients[nb_clients].sockfd = clientfd;
    memcpy(&clients[nb_clients].addr, &client_addr, sizeof(client_addr));
    nb_clients++;

    // Thread pour communiquer avec client
    int *arg = malloc(sizeof(*arg));
    if (arg == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    *arg = nb_clients - 1;
    
    if (pthread_create(&tid[nb_clients - 1], NULL, connexion_client, arg) != 0) {
      perror("pthread_create");
      exit(EXIT_FAILURE);
    }
  }
  return 0; 
}
