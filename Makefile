CC	= gcc
CFLAGS	= -Wall -O2

all: netstore_server netstore_client

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^

netstore_server: util.o server.c
	$(CC) $(CFLAGS) -o $@ $^

netstore_client: util.o client.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf netstore_client netstore_server *.o dto.h.gch util.h.gch