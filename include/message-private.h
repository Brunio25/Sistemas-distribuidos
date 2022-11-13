// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include <arpa/inet.h>

//tamanho máximo da mensagem enviada pelo cliente
#define MAX_MSG 2048 

int write_all(int sock, void *buf, int len);

int read_all(int sock, void *buf, int len);

#endif