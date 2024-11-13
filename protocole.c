#include "protocole.h"

void	reverse(char buffer[], int i, int j){
	char tmp;
	while (i < j){
		tmp = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = tmp;
		i++;
		j--;
	}
}

voidft_itoa(char nombre[], int n){
	int		tmp,i,r;

	tmp = n;
	i = 0;
	while (tmp > 0){
		r = tmp % 10;
		nombre[i] = '0' + r;
		tmp = tmp / 10;
		i++;
	}
	if (i == 0){
		nombre[i] = '0';
		i++;
	}
	nombre[i] = '\0';
	reverse(nombre, 0, i-1);
}

void ft_strcpy(char dest[], char src[]){
	int	i;

	i = 0;
	while (src[i] != '\0' && src[i] != '\n'){
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

void parse_1box(char buffer[], char box1[]){
	int i,j;

	i = TAG_SIZE + 1;
	j = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n'){
		box1[j] = buffer[i];
		i++;
		j++;
	}
	box1[j] = '\0';
}

void parse_2box(char buffer[], char box1[], char box2[]){
	int i,j;

	i = TAG_SIZE + 1;
	j = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != ' '){
		box1[j] = buffer[i];
		i++;
		j++;
	}
	box1[j] = '\0';
	i++;
	j = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n'){
		box2[j] = buffer[i];
		i++;
		j++;
	}
	box2[j] = '\0';
}

void parse_3box(char buffer[], char box1[], char box2[], char box3[]){
	int i,j;
	i = TAG_SIZE+1;
	j = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n' && buffer[i] != ' '){
		box1[j] = buffer[i];
		i++;
		j++;
	}
	box1[j] = '\0';
	i++;
	j = 0;
	while (buffer[i] != '\0'&& buffer[i] != '\n' && buffer[i] != ' '){
		box2[j] = buffer[i];
		i++;
		j++;
	}
	box2[j] = '\0';
	i++;
	j = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n'){
		box3[j] = buffer[i];
		i++;
		j++;
	}
	box3[j] = '\0';
}

int	make_request_1box(char buffer[], char box1[], char tag[]){
	int	i,j;

	i = 0;
	while (tag[i] != '\0' && i < 4){
		buffer[i] = tag[i];
		i++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-1 && box1[j] != '\0'){
		buffer[i] = box1[j];
		i++;
		j++;
	}
	buffer[i] = '\n';
	buffer[i+1] = '\0';
	return i+2;
}

int	make_request_2box(char buffer[], char box1[], char box2[], char tag[]){
	int	i,j;
	i = 0;
	while (tag[i] != '\0' && i < 4){
		buffer[i] = tag[i];
		i++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-2 && box1[j] != '\0'){
		buffer[i] = box1[j];
		i++;
		j++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-1 && box2[j] != '\0'){
		buffer[i] = box2[j];
		i++;
		j++;
	}
	buffer[i] = '\n';
	buffer[i+1] = '\0';
	return i+2;
}

int	make_request_3box(char buffer[], char box1[], char box2[], char box3[], char tag[]){
	int	i,j;

	i = 0;
	while (tag[i] != '\0' && i < 4){
		buffer[i] = tag[i];
		i++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-3 && box1[j] != '\0'){
		buffer[i] = box1[j];
		i++;
		j++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-2 && box2[j] != '\0'){
		buffer[i] = box2[j];
		i++;
		j++;
	}
	buffer[i] = ' ';
	i++;
	j = 0;
	while (i < BUFFER_SIZE-1 && box3[j] != '\0'){
		buffer[i] = box3[j];
		i++;
		j++;
	}
	buffer[i] = '\n';
	buffer[i+1] = '\0';
	return i+2;
}
