# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472

CC = gcc
LD= ld
INC_DIR = include
OBJ_DIR = object
BIN_DIR = binary
SRC_DIR = source
LIB_DIR = lib

#tree-server
tree-server = tree_server.o data.o entry.o tree.o network_server.o message.o #tree_skel.o sdmessage.pb-c.o 
#tree-client
tree-client = tree_client.o data.o entry.o client_stub.o network_client.o message.o #sdmessage.pb-c.o

client-lib = client_stub.o network_client.o data.o entry.o

OBJETOS = client-lib.o tree-client.o tree-server.o

CFLAGS = -g -Wall -I

all: client-lib.o tree-client tree-server

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC_DIR) -o $(OBJ_DIR)/$@ -c $<

client-lib.o: $(client-lib)
			ld -r $(addprefix $(OBJ_DIR)/,$^) -o $(LIB_DIR)/$@

tree-client: $(tree-client)
	$(CC)  $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -o $(BIN_DIR)/$@

tree-server: $(tree-server)
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -o $(BIN_DIR)/$@


clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*
	rm -f $(LIB_DIR)/*
    