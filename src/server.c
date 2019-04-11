
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>  // strlen

#include <pthread.h>
#include <sys/socket.h> 
#include <netinet/in.h> 

#define PORT 1042

struct client_args {
    int sock;
};

void* client_handler(void *vargp) {
    
    // TODO: Loop input
    // Call other functions for options
    
    struct client_args* args = (struct client_args*) vargp;
    int sock = args->sock;
    int valread;
    
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    send(sock, hello, strlen(hello), 0 );
    printf("Hello message sent\n");
    
    return NULL;
}

int main(int argc, char const *argv[]) { 
    
    // TODO: Make port be a command line argument
    
    int server_fd;
    struct sockaddr_in address;
    int len = sizeof(address);
    
    // Shorthand for passing arguments
    #define THIS server_fd, (struct sockaddr*) &address
    
    // Create server sock
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Initialize server address information
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(THIS, len) < 0) { 
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 32) < 0) { 
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Here I create a bunch of clients. The clients should
    // be able to terminate themselves, but..
    // 1) What happens if the client connection breaks?
    // 2) Can I prevent this server from accepting too many clients?
    // 3) #clients == #threads?
    int sock;
    pthread_t tid;
    struct client_args args;
    handle_next_client:
    
    if ((sock = accept(THIS, (socklen_t*) &len)) < 0) { 
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    args.sock = sock;
    pthread_create(&tid, NULL, client_handler, &args);
    goto handle_next_client;
    
    return 0;
}

// This waits for the thread (I think). I don't
// this this is needed since I'm not computing
// anything.
//void* status
//pthread_join(tid, &status);

