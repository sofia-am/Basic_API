CC = gcc 
CFLAGS = -g -Wall -pedantic -Wextra -std=gnu11 -Wconversion -Werror 

all: main

#	TODO: agregar a lib_misc la compilación de server_config

main.o: test.c
	$(CC) $(CFLAGS) -c -lulfius $< -o $@

main: main.o
	$(CC) $< -lulfius -o $@

clean:
	rm -rf main main.o