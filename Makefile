# // Grupo 4
# // Renato Custódio nº56320
# // Bruno Soares nº57100
# // Guilherme Marques nº55472

CC = gcc
CFLAGS = -g

SRCDIR = source
OBJDIR = object
BINDIR = binary
INCDIR = include

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
BIN = $(BINDIR)/out
INC = $(wildcard $(INCDIR)/*.h)


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ $(INCDIR)/*~ $(SRCDIR)/*~ $(BINDIR)/*
    