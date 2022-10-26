#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

//tamanho máximo da mensagem enviada pelo cliente
#define MAX_MSG 2048 

int write_all(int sock, char *buf, int len);

int read_all(int sock, char *buf, int len);

#endif