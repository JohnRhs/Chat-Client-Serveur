#include "client.h"
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#define SERVER_HOST "127.0.0.1"  
#define SERVER_PORT "8080" 
#define BUFFER_SIZE 1024

void handle_connection(int sockfd) {
  struct pollfd fds[2];
  char buffer[BUFFER_SIZE];
  int n;

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = sockfd;
  fds[1].events = POLLIN;

  // Attendre indéfiniment des événements sur les fd
  while (1) {
    poll(fds, 2, -1);

    // Si STDIN a des données
    if (fds[0].revents & POLLIN) {
      n = read(STDIN_FILENO, buffer, BUFFER_SIZE);
      if (n > 0) {
        // On envoie au socket
        send(sockfd, buffer, n, 0);
      }
    }

    // Si le socket a des données
    if (fds[1].revents & POLLIN) {
      // On récupère les données
      n = recv(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT);

      // Si on a reçu quelque chose
      if (n > 0) {
        if (strncmp(buffer, "PING\n\0", TAG_SIZE) == 0) {
          send(sockfd, "PONG\n\0", strlen("PONG\n\0"), 0);
        }
        // On écrit les données reçues sur la sortie standard
        write(STDOUT_FILENO, buffer, n);
      }
      // Sinon la connexion est fermée
      else {
        break;
      }
    }
  }
}

int main() {
  int sockfd;
  struct addrinfo hints, *res;

  // Initialisation de l'adresse du serveur
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Obtenir l'information de l'adresse du serveur
  if (getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  // Création du socket
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Connexion au serveur
  if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  printf("Connecté au serveur sur %s:%s\n", SERVER_HOST, SERVER_PORT);

  // Gérer la connexion
  handle_connection(sockfd);

  // Fermer le socket une fois terminé
  close(sockfd);
  freeaddrinfo(res);
  return 0;
}
