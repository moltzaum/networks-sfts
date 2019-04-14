
#include <stdio.h>  // perror, print, fgets
#include <stdlib.h> // exit, atoi
#include <stdbool.h>
#include <unistd.h> // read, write
#include <string.h> // ?
#include <errno.h>

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>

#define PORT 1042

void input_loop(int sock);

// Potential helper method?
void error(char *msg) {
    perror(msg);
    exit(1);
}

bool equals(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

bool prefix(const char* pre, const char* str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

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

// TODO: check errors

//do {
//    read(sock, buf, BUFSIZ);
//    printf("%s", buf);
//    read(sock, buf, BUFSIZ);
//} while (!equals(buf, "done"));

//  here <--- sock
void download(int sock, const char* filename) {

}

//  here ---> sock
void upload(int sock, const char* filename) {

}

void input_loop(int sock) {
    char buf[BUFSIZ];
    char *line = NULL;
    size_t len = 0;
    ssize_t n;
    
    while ((n = getline(&line, &len, stdin) != -1)) {
        
        // getline includes newline in result string 
        line[strlen(line)-1] = '\0';
        write(sock, line, len);
        
        // What is the action to perform?
        read(sock, buf, BUFSIZ);
        
        // print is used for either an error or information
        // since we never quit on an error, there we make no distinction
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
        
        if (prefix("download", line)) {
            read(sock, buf, BUFSIZ);
            download(sock, buf);
            continue;
        }
        
        if (prefix("upload", line)) {
            read(sock, buf, BUFSIZ);
            upload(sock, buf);
            continue;
        }
        
        // The server sent a bad response
        // Let the server know it was a bad response
        printf("error: invalid response from server\n");
        printf("%s", buf);
        const char* msg = "invalid action to perform";
        const char* log = "log";
        printf("DEBUG: sent log\n");
        write(sock, log, strlen(log));
        write(sock, msg, strlen(msg));
        
    }
    free(line);
    
}

//int i;
//asm("movl %%eax, %0" : "=r" (i));
//printf("The value of eax is %d\n", i);
// SetErrorTestType(INT_SENTINEL)
// SetErrorTestType(ERRNO)
