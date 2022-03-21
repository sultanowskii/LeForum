#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/time.h>

#include "lib/constants.h"
#include "lib/error.h"
#include "lib/communication.h"
#include "lib/queue.h"
#include "lib/forum.h"


int32_t SERVER_PORT = 7431;
char SERVER_ADDR[] = "0.0.0.0";
int32_t MAX_CONNECTIONS = 100;
struct timeval TIMEOUT = {3, 0};

/*
 * Query queues
 * 
 * Please note that they have to contain pointers to LeThreads.
 */
struct Queue *lethread_query_queue;
struct Queue *lemessage_query_queue;
struct Queue *leauthor_query_queue;

/*
 * To free() all the LeClientInfo structs in the end.
 */
struct Queue *leclientinfo_queue;

/*
 * handle_client() argument
 */
struct LeClientInfo {
	int32_t fd;
	socklen_t addr_size;
	struct sockaddr_in addr;
};

/*
 * Saves LeThreads (to corresponding files). This function has to be run
 * in the separate thread.
 * 
 * The purpose is to avoid accessing same file from different threads.
 */
void * lethread_query_manage() {
	while (TRUE) {
		while (!queue_is_empty(lethread_query_queue)) {
			lethread_save(queue_pop(lethread_query_queue));
		}
	}
}

/*
 * Saves LeMessages (to corresponding files). This function has to be run
 * in the separate thread.
 * 
 * The purpose is to avoid accessing same file from different threads.
 */
void * lemessage_query_manage() {
	while (TRUE) {
		while (!queue_is_empty(lemessage_query_queue)) {
			lemessages_save(queue_pop(lemessage_query_queue));
		}
	}
}

/*
 * Saves LeAuthors (to corresponding files). This function has to be run
 * in the separate thread.
 * 
 * The purpose is to avoid accessing same file from different threads.
 */
void * leauthor_query_manage() {
	while (TRUE) {
		while (!queue_is_empty(leauthor_query_queue)) {
			leauthor_save(queue_pop(leauthor_query_queue));
		}
	}
}

/*
 * Used by queue_delete()
 */
void leclientinfo_delete(struct LeClientInfo *clinfo) {
	free(clinfo);
}

/*
 * Communicates with a client, gets and sends queries
 * and requests.
 */
void * handle_client(void *arg) {
	struct LeClientInfo *client_info = (struct LeClientInfo *)arg;
	
	char cl_data[9 * 1024];
	char sv_data[9 * 1024];

	int64_t cl_data_size = 0;
	int64_t sv_data_size = 0;
	
	/* =================================== Example ====================================== */
	char client_ip[128];

	uint16_t client_port = ntohs(client_info->addr.sin_port);
	inet_ntop(AF_INET, &(client_info->addr.sin_addr), client_ip, 128);

	sendf(client_info->fd, "Hi! You are fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);
	printf("Connection from fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);

	/* ================================= Example end ==================================== */

	while (TRUE) {
		cl_data_size = recv(client_info->fd, cl_data, 9 * 1024 - 1, NULL);
		
		/* Timeout/connection closed */
		if (cl_data_size < 0) {
			break;
		}
	}

	close(client_info->fd);
}


int32_t main(int32_t argc, char *argv[]) {
	int32_t client_fd, server_fd;
	struct sockaddr_in server_addr;
	struct sockaddr client_addr;
	socklen_t client_addr_len;
	socklen_t socakddr_in_len = sizeof(struct sockaddr_in);
	
	pthread_t client_handler_thread;
	pthread_t lethread_query_manager_thread;
	pthread_t lemessage_query_manager_thread;
	pthread_t leauthor_query_manager_thread;

	struct LeClientInfo *leclientinfo;

	lethread_query_queue = queue_create();
	lemessage_query_queue = queue_create();
	leauthor_query_queue = queue_create();
	leclientinfo_queue = queue_create();

	puts("LeForum Server");

	if (pthread_create(&lethread_query_manager_thread, NULL, lethread_query_manage, NULL) != 0) {
		perror("failed to start lethread query manager");
		return ERRCLIB;
	}

	if (pthread_create(&lemessage_query_manager_thread, NULL, lemessage_query_manage, NULL) != 0) {
		perror("failed to start lemessage query manager");
		return ERRCLIB;
	}

	if (pthread_create(&leauthor_query_manager_thread, NULL, leauthor_query_manage, NULL) != 0) {
		perror("failed to start leauthor query manager");
		return ERRCLIB;
	}

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

		leclientinfo = malloc(sizeof(struct LeClientInfo));
		leclientinfo->fd = client_fd;

		if (getpeername(client_fd, &leclientinfo->addr, &socakddr_in_len) < 0) {
			perror("getpeername()");
			return ERRCLIB;
		}

		queue_push(leclientinfo_queue, leclientinfo, sizeof(leclientinfo));

		if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&TIMEOUT, sizeof(TIMEOUT)) < 0) {
			perror("setsockopt() failed");
			return ERRCLIB;
		}

		if (pthread_create(&client_handler_thread, NULL, handle_client, (void*)leclientinfo) != 0) {
			perror("failed to create client handle");
			return ERRCLIB;
		}
	}

	queue_delete(leclientinfo_queue, leclientinfo_delete);

	return 0;
}
