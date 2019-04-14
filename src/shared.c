
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

void error(char* msg) {
    perror(msg);
    exit(1);
}

bool equals(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool send_permissions(int sock, int fd) {
    char buf[BUFSIZ];
    struct stat s;
    
    fstat(fd, &s);
    sprintf(buf, "%d", s.st_uid);
    write(sock, buf, BUFSIZ);
    sprintf(buf, "%d", s.st_gid);
    write(sock, buf, BUFSIZ);
    sprintf(buf, "%d", s.st_mode);
    write(sock, buf, BUFSIZ);
    return true;
}

bool get_permissions(int sock, int fd) {
    
    char buf[BUFSIZ];
    int gid, uid, mode;
    
    read(sock, buf, BUFSIZ);
    gid = atoi(buf);
    read(sock, buf, BUFSIZ);
    uid = atoi(buf);
    read(sock, buf, BUFSIZ);
    mode = atoi(buf);
    
    fchown(fd, uid, gid);
    fchmod(fd, mode);
    return true;
}

bool read_file_write_sock(int sock, FILE* file) {
    
    char buf[BUFSIZ];
    char bytes_read[BUFSIZ];
    
    int n = fread(buf, 1, BUFSIZ, file);
    do {
        sprintf(bytes_read, "%d", n);
        write(sock, bytes_read, BUFSIZ);
        write(sock, buf, BUFSIZ);
    } while ((n = fread(buf, 1, BUFSIZ, file)) != 0);
    fclose(file);
    return true;
}

bool read_sock_write_file(int sock, FILE* file) {
    
    char buf[BUFSIZ];
    char bytes_read[BUFSIZ];
    
    int n = 0;
    do {
        read(sock, bytes_read, BUFSIZ);
        n = atoi(bytes_read);
        read(sock, buf, BUFSIZ);
        fwrite(buf, 1, n, file);
    } while (n == BUFSIZ);
    fclose(file);
    return true;
}

