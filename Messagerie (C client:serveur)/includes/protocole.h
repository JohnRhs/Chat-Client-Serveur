#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#define TAG_SIZE 4
#define BUFFER_SIZE 1024

void	reverse(char buffer[], int i, int j);
void	ft_itoa(char nombre[], int n);
void	ft_strcpy(char dest[], char src[]);

void	parse_1box(char buffer[], char box1[]);
void	parse_2box(char buffer[], char box1[], char box2[]);
void	parse_3box(char buffer[], char box1[], char box2[], char box3[]);

int		make_request_1box(char buffer[], char box1[], char tag[]);
int		make_request_2box(char buffer[], char box1[], char box2[], char tag[]);
int		make_request_3box(char buffer[], char box1[], char box2[], char box3[], char tag[]);

#endif