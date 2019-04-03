
all: server client

bin:
	mkdir bin

server: bin
	gcc src/server.c -o bin/server

client: bin
	gcc src/client.c -o bin/client

clean:
	rm -f bin/client
	rm -f bin/server
