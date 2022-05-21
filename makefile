CC = gcc 
CFLAGS = -g -Wall -pedantic -Wextra -std=gnu11 -Wconversion #-Werror 
OBJS = obj#nombre de la carpeta donde se guardan los .o
SRC = src
BIN = bin
INC = inc
LIB = lib
DEP = inc/dependencies.h
WHERE = -Wl,-rpath,.
CONF = config_files
NGINX = /etc/nginx/conf.d
all: mkdir nginx server.service main 
#TODO: escribir los archivos de configuracion en las ubicaciones correspondientes
mkdir:
	mkdir -p $(SRC) $(BIN) $(LIB) $(OBJS) $(INC)

nginx:
	cd $(CONF) \
	sudo mkdir --parents .$(NGINX) \
	sudo cp nginx_config.conf $(NGINX)\
	sudo nginx -t \
	sudo systemctl reload nginx.service

server.service:
	cd $(CONF) \
	sudo cp $@ /etc/systemd/system \
	sudo systemctl daemon-reload \
	sudo systemctl enable $@ &\
	sudo systemctl start $@ &\

main.o: $(SRC)/test.c
	$(CC) $(CFLAGS) -c $< -o $(OBJS)/$@

main: main.o
	$(CC) $(OBJS)/$< -lulfius -ljansson -lyder -o $@

clean:
	rm -rf main main.o