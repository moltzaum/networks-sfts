
all: server client
	./bin/server

bin:
	mkdir bin

server: bin
	gcc -x c src/shared.c src/server.c -o bin/server

client: bin
	gcc -x c src/shared.c src/client.c -o bin/client

clean:
	rm -f bin/client
	rm -f bin/server
