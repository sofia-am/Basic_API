CC = gcc 
CFLAGS = -g -Wall -pedantic -Wextra -std=gnu11 -Wconversion #-Werror 
OBJS = obj#nombre de la carpeta donde se guardan los .o
SRC = src
BIN = bin
INC = inc
LIB = lib
DEP = inc/dependencies.h
WHERE = -Wl,-rpath,.

all: mkdir main
#TODO: escribir los archivos de configuracion en las ubicaciones correspondientes
mkdir:
	mkdir -p $(SRC) $(BIN) $(LIB) $(OBJS) $(INC)

main.o: $(SRC)/test.c
	$(CC) $(CFLAGS) -c $< -o $(OBJS)/$@

main: main.o
	$(CC) $(OBJS)/$< -lulfius -ljansson -lyder -o $@

clean:
	rm -rf main main.o