#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    char *str = malloc(20);
    memcpy(str, "123:456", sizeof("123:456"));
    char *c = strtok(str, ":");
    printf("%s\n", c);
    printf("%s", strtok(NULL, ":"));
    free(str);
}