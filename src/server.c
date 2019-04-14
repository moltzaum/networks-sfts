
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
void download(int sock, const char* src, const char* dst) {

    char buf[BUFSIZ];
    FILE* file = fopen(src, "r");
    if (file) {
        write(sock, "download", BUFSIZ);
        write(sock, dst, BUFSIZ);
        while (fgets(buf, BUFSIZ, file) != NULL) {
            write(sock, buf, BUFSIZ);
        }
        write(sock, "done", BUFSIZ);
        fclose(file);
    }
}

//  here <--- sock
void upload(int sock, const char* src, const char* dst) {
    printf("DEBUG: inside upload!!\n");
    
    write(sock, "none", BUFSIZ);
}

void runCommand(int sock, const char* cmd) {
    
    char buf[BUFSIZ] = {};
    FILE* result = popen(cmd, "r");
    if (result == NULL) { // error..
        return;
    }
    write(sock, "print", BUFSIZ);
    while (fgets(buf, BUFSIZ, result) != NULL) 
        write(sock, buf, BUFSIZ);
    write(sock, "done", BUFSIZ);
    pclose(result);
}

void* client_handler(void *vargp) {
    
    struct client_args* handler_args = (struct client_args*) vargp;
    int sock = handler_args->sock;
    int n;
    
    char* args;
    char buf[BUFSIZ] = {};
    char cmd[BUFSIZ] = {};
    
    while ((n = read(sock, buf, BUFSIZ) != -1)) {
         
        memset(cmd, 0, BUFSIZ); // loop reset
        printf("DEBUG: The line is: '%s'\n", buf);
        
        char* args = strpbrk(buf, " ");
        
        if (equals("catalog", buf) || prefix("catalog ", buf)) { 
            // Allows for args. I should sanitize the input
            //strcpy(cmd, "ls");
            //if (args) strcat(cmd, args);
            //runCommand(sock, cmd);
            runCommand(sock, "ls");
            continue;
            
        } else if (equals("spwd", buf)) {
            runCommand(sock, "pwd");
            continue;
        }
        
        if (equals("bye", buf)) {
            write(sock, "print", BUFSIZ);
            write(sock, "File copy server is down!", BUFSIZ);
            write(sock, "done", BUFSIZ);
            close(sock);
            exit(0);
        }
         
        // We only expect two arguments
        char* src = strtok(args, " ");
        char* dst = strtok(NULL, " ");
        char* trd = strtok(NULL, " ");
        
        const char* usage; 
        typedef void transfer_func(int, char*, char*);
        void (*transfer)(int, char*, char*);
        
        if (prefix("download", buf)) {
            transfer = (transfer_func*) &download;
            usage = "usage: download <src> <dst>\n"; 
             
        } else if (prefix("upload", buf)) {
            transfer = (transfer_func*) &upload;
            usage = "usage: upload <src> <dst>\n"; 
            
        } else if (equals("", buf)) {
            write(sock, "none", BUFSIZ);
            continue;
            
        } else {
            write(sock, "print", BUFSIZ);
            write(sock, "error: command not recognized\n", BUFSIZ);
            write(sock, "done", BUFSIZ);
            continue;
        }
        
        // Incorrect arguments to transfer function
        if (src == NULL || dst == NULL || trd != NULL) {
            write(sock, "print", BUFSIZ);
            write(sock, usage, BUFSIZ);
            write(sock, "done", BUFSIZ);
            continue;
        }
        transfer(sock, src, dst);
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

