// Client side C/C++ program to demonstrate Socket programming 

// Note: A good reference would be 'Unix Network Programming Volume 1'

#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <errno.h>

#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 

#define PORT 1042

int main(int argc, char const *argv[])
{ 
	struct sockaddr_in address;
	struct sockaddr_in serv_addr;
	int sock = 0, valread;
	char *hello = "Hello from client";
	char buffer[1024] = {0};

  // Initialize the socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("Socket creation error\n");
		return -1;
	}

  // Initialize Server Information
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
  // argv[1] can be ip
  // argv[1] can be hostname
  // can hostname be something from /etc/hosts?
  // 
  //server = gethostbyname(argv[1]);
  //if server == NULL
  //  fprintf(stderr, "ERROR..."
  //
  // server->h_name is server hostname and can be used in inet_pton
  // server->h_addr is server address and can be set to serv_addr.sin_addr.s_addr
  //

	// Convert IPv4 and IPv6 addresses from text to binary form 
  //
  // Note: inet_pton supports ipv4 and ipv6, but doesn't support ipv4
  // shorthand like inet_aton, which doesn't support ipv6.

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { 
		printf("Invalid address/ Address not supported\n");
		return -1;
	}

  // Connect to sock
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
		printf("Connection Failed \n");
		return -1;
	}

	write(sock, hello, strlen(hello));
	printf("Hello message sent\n");
	valread = read(sock , buffer, 1024);

  // I can get the value returned from eax for some cases
  // This is not the case for odd return sizes
  // It does not work on Windows (I think)
  // It does not work on inline functions
  int i;
  asm("movl %%eax, %0" : "=r" (i));
  printf("The value of eax is %d\n", i);
  //printf("The number of bytes read is %d\n", valread);
  //
  // SetErrorTestType(INT_SENTINEL)
  // SetErrorTestType(ERRNO)
  //    might be able to set to 0 after handling

  // valread = bytes read, errno is set to != 0
  // for safety, check valread to know if errno was updated instead of
  // trying to say errno = 0 before each function that _may_ update the errno
  // I can simply use perror, or use strerror(errno) in a formatted string
	printf("%s\n", buffer);
	return 0;
}
