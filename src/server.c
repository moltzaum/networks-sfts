
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <string.h> // strlen

#include <pthread.h>
#include <sys/socket.h> 
#include <netinet/in.h> 

#define PORT 1042

struct client_args {
    int sock;
};

bool equals(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

//  here ---> sock 
void download(int sock, const char* filename) {
    
}

//  here <--- sock
void upload(int sock, const char* filename) {
    
}

void* client_handler(void *vargp) {
    
    struct client_args* handler_args = (struct client_args*) vargp;
    int sock = handler_args->sock;
    int n;
    
    char* args;
    bool doCmd = false;
    char buf[BUFSIZ] = {};
    char cmd[BUFSIZ] = {};
    
    #define PRINT write(sock, "print", BUFSIZ)
    #define DONE write(sock, "done", BUFSIZ)
    
    while ((n = read(sock, buf, BUFSIZ) != -1)) {
         
        // Reset variables
        doCmd = false;
        memset(cmd, 0, BUFSIZ);
        
        printf("DEBUG: The line is: '%s'\n", buf);
        
        if (equals("log", buf)) {
            read(sock, buf, BUFSIZ);
            printf("DEBUG: log: %s\n", buf);
        }
        
        if (equals("catalog", buf) || prefix("catalog ", buf)) {
            strcpy(cmd, "ls");
            doCmd = true;
             
        } else if (equals("ls", buf) || prefix("ls ", buf)) {
            strcpy(cmd, "ls");
            doCmd = true;
             
        } else if (equals("spwd", buf) || prefix("spwd ", buf)) {
            strcpy(cmd, "pwd");
            doCmd = true;
             
        } else if (equals("bye", buf) || prefix("bye ", buf)) {
            PRINT;
            write(sock, "File copy server is down!", BUFSIZ);
            DONE;
            close(sock);
            exit(0);
        }
         
        // Separate the command from the args, then
        // concatenate the args to the "real" command
        // BTW, this is vulnerable to attack. I should sanitize the input
        args = strpbrk(buf, " ");
        if (args && doCmd) {
            strcat(cmd, args);
        }
        
        // Put into Function?
        // cmd, buf
        printf("command is: %s", cmd);
        if (doCmd) {
            printf("DEBUG: The command is: %s\n", cmd);
            FILE* out;
            if ((out = popen(cmd, "r")) != NULL) {
                PRINT;
                while (fgets(buf, BUFSIZ, out) != NULL) {
                    printf("DEBUG: %s", buf);
                    write(sock, buf, BUFSIZ);
                }
                DONE;
            } 
            pclose(out);
            continue;
        }
        
        // We only expect two arguments
        // If I get a bus error, change char* to char[] for args?
        char* src = strtok(args, " ");
        char* dst = strtok(NULL, " ");
        
        printf("src: %s\n", src);
        printf("dst: %s\n", dst);
        
        if (prefix("download", buf)) {
            //download(sock, );
            continue;
             
        } else if (prefix("upload", buf)) {
            //upload(sock, );
            continue;
            
        } else {
            const char* msg = "error: command not recognized\n";
            PRINT;
            write(sock, msg, BUFSIZ);
            DONE;
            continue;
        }
    }
    
    return NULL;
}

int main(int argc, char const *argv[]) { 
    
    // TODO: Make port be a cmd line argument
    
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

