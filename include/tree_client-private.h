// Grupo 4
// Renato Custódio nº56320
// Bruno Soares nº57100
// Guilherme Marques nº55472

#ifndef _TREE_CLIENT_PRIVATE_H
#define _TREE_CLIENT_PRIVATE_H

//prints all the keys frem an array
void printKeys(char **strs);

//prints all the values frem an array
void printValues(void **values);

//prints the commands the program accepts
void usage();

//destroys an array of keys
void free_keys(char **keys);

//destroys an array of values
void free_values(void **values);


#endif