CC	= gcc
CFLAGS	= -Wall -O2

all: netstore_server netstore_client

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

dto.o: dto.h
	$(CC) $(CFLAGS) -c dto.h

netstore_server: util.o dto.o server.c
	$(CC) $(CFLAGS) -c server.c

netstore_client: util.o dto.o client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -rf netstore-client netstore-server *.o