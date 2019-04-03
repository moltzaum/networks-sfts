
all: server client

server:
	gcc src/server.c -o bin/server

client:
	gcc src/client.c -o bin/client

clean:
	rm client
	rm server
