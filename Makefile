all: relay-server client

relay-server: relay-server.c
	gcc -o relay-server relay-server.c

client: client.c
	gcc -o client client.c
clean:
	rm -f *.o relay-server client

