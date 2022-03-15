#include <stdio.h> // i/o
#include <stdlib.h> // default functions
#include <string.h> // strings, memory

#include <unistd.h> // uni calls

#include <pthread.h> // threading

#include <sys/socket.h> // sockets
#include <arpa/inet.h> // inet_addr

#include "lib/types.h"
#include "lib/error.h"
#include "lib/communication.h"


int32_t SERVER_PORT = 7431;
char SERVER_ADDR[] = "0.0.0.0";
int32_t MAX_CONNECTIONS = 100;


void* handle_client(void* arg) {
	char buffer[256];
	int32_t fd = *(int32_t *)arg;

	sendf(fd, "Hi! fd=%d\n", fd);
	printf("Hi! fd=%d\n", fd);

	close(fd);
}


int32_t main(int32_t argc, char* argv[]) {
	int32_t client_fd, server_fd;
	struct sockaddr_in server_addr;
	struct sockaddr client_addr;
	socklen_t client_addr_len;
	pthread_t client_thread;

	puts("LeForum Server");

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		perror("socket() failed");
		return ERRCLIB;
	}

	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		perror("bind() failed");
		return ERRCLIB;
	}

	if (listen(server_fd, MAX_CONNECTIONS) != 0) {
		perror("listen() failed");
		return ERRCLIB;
	}

	while (TRUE) {
		client_fd = accept(server_fd, &client_addr, &client_addr_len);
		
		if (client_fd < 0) {
			perror("accept() failed");
			return ERRCLIB;
		}
		
		// TODO: pass client_fd, client_addr, client_addr_len as arguments.
		if (pthread_create(&client_thread, NULL, handle_client, (void*)&client_fd) != 0) {
			perror("failed to create client handle");
			return ERRCLIB;
		}
	}

	return 0;
}
