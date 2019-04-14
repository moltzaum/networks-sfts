
#include <stdio.h>  // perror, print, fgets
#include <stdlib.h> // exit, atoi
#include <stdbool.h>
#include <unistd.h> // read, write
#include <string.h> // ?
#include <errno.h>

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/stat.h>

#include "shared.h"

#define PORT 1042

void input_loop(int sock);

int main(int argc, char const *argv[]) { 
    
    struct sockaddr_in address;
    struct sockaddr_in serv_addr;
    int sock = 0;
    char buffer[1024] = {0};
    
    /*
    int portno;
    struct hostent *server;
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
    // Fetch Port and Hostname from Args
    // argv[1] can be ip
    // argv[1] can be hostname
    // can hostname be something from /etc/hosts?
    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    */
    // server->h_name is server hostname and can be used in inet_pton
    // server->h_addr is server address and can be set to serv_addr.sin_addr.s_addr
    
    // Initialize the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Socket creation error\n");
        return -1;
    }
    
    // START SERV_ADDR STUFF (also related to above)
    
    // Initialize Server Information
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    //
    // Note: inet_pton supports ipv4 and ipv6, but doesn't support ipv4
    // shorthand like inet_aton, which doesn't support ipv6.
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { 
        printf("Invalid address/ Address not supported\n");
        return -1;
    }
    
    // END SERV_ADDR STUFF
    
    // Connect to sock
    if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) { 
        printf("Connection Failed \n");
        return -1;
    }
    
    input_loop(sock);
    return 0;
}

//  here <--- sock
bool download(int sock) {
    
    char dst[BUFSIZ];
    read(sock, dst, BUFSIZ);
    FILE* file = fopen(dst, "wb");
    if (file) {
        get_permissions(sock, fileno(file));
        read_sock_write_file(sock, file);
    } else {
        printf("DEBUG: client download file not found?\n");
    }
    return file;
}

//  here ---> sock
bool upload(int sock) {
    
    char src[BUFSIZ];
    read(sock, src, BUFSIZ);
    FILE* file = fopen(src, "rb");
    if (file) {
        send_permissions(sock, fileno(file));
        read_file_write_sock(sock, file);
    } else {
        printf("DEBUG: client upload file not found?\n");
    }
    return file;
}

ssize_t line_prompt(char** line, size_t* len, FILE* stream) {
    printf("> ");
    return getline(line, len, stream); 
}

void input_loop(int sock) {
    char buf[BUFSIZ];
    char *line = NULL;
    size_t len = 0;
    ssize_t n;
    
    while ((n = line_prompt(&line, &len, stdin) != -1)) {
        
        // getline includes newline in result string 
        line[strlen(line)-1] = '\0';

        if (equals(line, "ls")) {
            system("ls");
            continue;
        }

        if (equals(line, "pwd")) {
            system("pwd"); 
            continue;
        }

        write(sock, line, len);
        
        // What is the action to perform?
        read(sock, buf, BUFSIZ);
        
        if (equals(buf, "none")) {
            continue;
        }

        // print is used for either an error or information
        // since we never quit on an error, we make no distinction
        if (equals(buf, "print")) {
            
            continue_print:
                read(sock, buf, BUFSIZ);
                if (equals(buf, "done")) 
                    continue;
                printf("%s", buf);
            goto continue_print;
        } 
        
        if (equals(buf, "bye")) {
            printf("Internet copy client is down!");
            // DEALLOCATE RESOURCES!!
            return;
        }
        
        if (equals("download", buf)) {
            download(sock);
            continue;
        }
        
        if (equals("upload", buf)) {
            upload(sock);
            continue;
        }
        
        // The server sent a bad response
        // Let the server know it was a bad response
        printf("error: invalid response from server\n");
        printf("DEBUG: %s", buf);
        
    }
    free(line);
    
}

//int i;
//asm("movl %%eax, %0" : "=r" (i));
//printf("The value of eax is %d\n", i);
// SetErrorTestType(INT_SENTINEL)
// SetErrorTestType(ERRNO)
