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

void connexion(int sockfd) {
  struct pollfd fds[2];
  char buffer[1024];
  int nb_bits;

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = sockfd;
  fds[1].events = POLLIN;

  while (1) {
    poll(fds, 2, -1);

    if (fds[0].revents & POLLIN) {
      nb_bits = read(STDIN_FILENO, buffer, 1024);
      if (nb_bits > 0){
        send(sockfd, buffer, nb_bits, 0);
      }
    }

    if (fds[1].revents & POLLIN) {
      nb_bits = recv(sockfd, buffer, 1024, 0);
      if (nb_bits > 0) {
        write(STDOUT_FILENO, buffer, nb_bits);
      } else { 
        break; 
      }
    }
  }
}

int main(int argc, char *argv[]){
  if (argc == 2) { 
    if (strcmp(argv[1], "P8") == 0){
      int sockfd;
      struct addrinfo hints, *res;

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;

      if(getaddrinfo("127.0.0.1", "5678", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE); 
      }

      sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

      if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("connect");
        exit(EXIT_FAILURE); 
      }
      printf("> -- P8 CHATROOM -- \n");
      printf("COMMANDES : NAME <pseudo> | CRGP <taille> <groupe> | JOIN <groupe> | QUIT <groupe> | PRIV <pseudo> <message> | LIST | GRPL | MEMB <groupe> | EXIT |\n\n");
      connexion(sockfd);
    }
  }
  else {
    fprintf(stderr, "Usage: %s P8\n", argv[0]);
    exit(EXIT_FAILURE); 
  }
}