#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "lib/constants.h"
#include "lib/error.h"
#include "lib/communication.h"


int32_t SERVER_PORT = 7431;
char SERVER_ADDR[] = "0.0.0.0";
int32_t MAX_CONNECTIONS = 100;


struct LeClientInfo {
	int32_t fd;
	socklen_t addr_len;
	struct sockaddr *addr;
};

/*
 * Communicates with a client, gets and sends queries
 * and requests.
 */
void * handle_client(void *arg) {
	char buffer[256];
	char tmp[128];
	struct LeClientInfo *client_info = (struct LeClientInfo *)arg;

	inet_ntop(AF_INET, &(((struct sockaddr_in *)client_info->addr)->sin_addr), tmp, 128);
	sendf(client_info->fd, "Hi! fd=%d, addr=%s\n", client_info->fd, tmp);
	printf("Hi! fd=%d addr=%s\n", client_info->fd, tmp);

	close(client_info->fd);
}


int32_t main(int32_t argc, char *argv[]) {
	int32_t client_fd, server_fd;
	struct sockaddr_in server_addr;
	struct sockaddr client_addr;
	socklen_t client_addr_len;
	pthread_t client_thread;

	struct LeClientInfo *client_info;

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

		client_info = malloc(sizeof(struct LeClientInfo));
		client_info->fd = client_fd;
		client_info->addr = malloc(sizeof(struct LeClientInfo));
		memcpy(client_info->addr, (void *)&client_addr, client_addr_len);
		client_info->addr_len = client_addr_len;
		
		if (pthread_create(&client_thread, NULL, handle_client, (void*)client_info) != 0) {
			perror("failed to create client handle");
			return ERRCLIB;
		}
	}

	return 0;
}
