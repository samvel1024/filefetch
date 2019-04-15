CC	= gcc
CFLAGS	= -Wall -O2

all: netstore_server netstore_client

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

dto.o: dto.h
	$(CC) $(CFLAGS) -o dto.o dto.h

netstore_server: util.o dto.o server.c
	$(CC) $(CFLAGS) -o netstore_server server.c util.o

netstore_client: util.o dto.o client.c
	$(CC) $(CFLAGS) -o netstore_client client.c util.o

clean:
	rm -rf netstore_client netstore_server *.o dto.h.gch