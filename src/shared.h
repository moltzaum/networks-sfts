
#ifndef SHARED_H
#define SHARED_H

void error(char* msg);

bool equals(const char* str1, const char* str2);
bool prefix(const char *pre, const char *str);

bool send_permissions(int sock, int fd);
bool get_permissions(int sock, int fd);

bool read_file_write_sock(int sock, FILE* file);
bool read_sock_write_file(int sock, FILE* file);

#undef BUFSIZ
#define BUFSIZ 1024

#endif

