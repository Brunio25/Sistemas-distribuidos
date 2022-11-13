# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472

CC = gcc

PROTOC_DIR=/usr/
CFLAGS = -Wall -g -O2 -I${PROTOC_DIR}include/
LDFLAGS = ${PROTOC_DIR}lib/x86_64-linux-gnu/libprotobuf-c.a

LD = ld
INC_DIR = include
OBJ_DIR = object
BIN_DIR = binary
SRC_DIR = source
LIB_DIR = lib


tree-server = tree_server.o data.o entry.o tree.o network_server.o message.o sdmessage.pb-c.o tree_skel.o 

tree-client = tree_client.o data.o entry.o client_stub.o network_client.o message.o sdmessage.pb-c.o

objects = tree_server.o data.o entry.o tree.o network_server.o message.o sdmessage.pb-c.o tree_skel.o tree_client.o network_client.o client_stub.o

client-lib = client_stub.o network_client.o data.o entry.o

CFLAGS = -g -Wall -I

all: client-lib.o tree-client tree-server

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC_DIR) -o $(OBJ_DIR)/$@ -c -I include $<

tree.o: $(OBJ_DIR)/tree.o

client-lib.o: $(client-lib)
			ld -r $(addprefix $(OBJ_DIR)/,$^) -o $(LIB_DIR)/$@

tree-client: $(tree-client)
	$(CC)  $(addprefix $(OBJ_DIR)/,$^) -o $(BIN_DIR)/$@ $(LDFLAGS)

tree-server: $(tree-server)
	$(CC) $(addprefix $(OBJ_DIR)/,$^) -o $(BIN_DIR)/$@ $(LDFLAGS)


clean:
	rm -f $(filter-out $(OBJ_DIR)/tree.o,$(addprefix $(OBJ_DIR)/,$(objects)))
	rm -f $(BIN_DIR)/*
	rm -f $(LIB_DIR)/*
    