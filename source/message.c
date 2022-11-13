#// Grupo 4
#// Renato Custódio nº56320
#// Bruno Soares nº57100
#// Guilherme Marques nº55472

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message-private.h"

int write_all(int sock, uint8_t *buf, int len) {
    int bufsize = len;
    while (len > 0) {
        int res = write(sock, buf, len);
        if (res < 0) {
            if (errno == EINTR) continue;
            perror("write failed:");
            return res;
        }
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, uint8_t *buf, int len) {
    int readBytes = 0;
    int result;

    while (readBytes < len) {
        result = read(sock, buf + readBytes, len - readBytes);
        if (result < 1) {
            return result;
        }
        readBytes += result;
    }
    return readBytes;
}