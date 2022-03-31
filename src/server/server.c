#include "server/server.h"


int32_t            SERVER_PORT              = 7431;
char               SERVER_ADDR[]            = "0.0.0.0";
int32_t            MAX_CONNECTIONS          = 100;
struct timeval     TIMEOUT                  = {3, 0};

/*
 * Flag for threads
 */
bool_t             program_on_finish        = FALSE;

/* 
 * Stores the value of the next created LeThread id
 */
uint64_t           next_lethread_id_value   = 0;

/*
 * Pthread mutexes
 */
pthread_mutex_t   *next_lethread_id_mutex;

/*
 * Save file query queues with the purpose of prevention data race..
 */
struct Queue      *lethread_query_queue;
struct Queue      *lemessages_query_queue;
struct Queue      *lemessage_query_queue;
struct Queue      *leauthor_query_queue;

/*
 * To free() all the LeClientInfo structs in the end.
 */
struct Queue      *leclientinfo_queue;

/*
 * Here we store all the LeThreads
 */
struct Queue      *lethread_queue;

/*
 * Saves LeThreads (to corresponding files). This function has to be run
 * in the separate thread.
 * 
 * The purpose is to avoid accessing same file from different threads.
 */
void * lethread_query_manage() {
	while (!program_on_finish) {
		while (!queue_is_empty(lethread_query_queue) && !program_on_finish) {
			lethread_save(queue_pop(lethread_query_queue));
		}
	}
}

void * lemessages_query_manage() {
	while (!program_on_finish) {
		while (!queue_is_empty(lemessages_query_queue) && !program_on_finish) {
			lemessages_save(queue_pop(lemessages_query_queue));
		}
	}
}

void * lemessage_query_manage() {
	while (!program_on_finish) {
		while (!queue_is_empty(lemessage_query_queue) && !program_on_finish) {
			lemessage_save(queue_pop(lemessage_query_queue));
		}
	}
}

void * leauthor_query_manage() {
	while (!program_on_finish) {
		while (!queue_is_empty(leauthor_query_queue) && !program_on_finish) {
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
 * Implementation of lethread_get_by_id() required by query.h
 */
struct LeThread * lethread_get_by_id(uint64_t lethread_id) {
	struct LeThread         *lethread;
	struct QueueNode        *node           = lethread_queue->first;


	while (node != NULL) {
		lethread = node->data;
		if (lethread->id == lethread_id) {
			goto SUCCESS;
		}
		node = node->next;
	}

	lethread = malloc(sizeof(struct LeThread));

	if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
		free(lethread);
		return LESTATUS_NFND;
	}

SUCCESS:
	if (lethread->messages->first == NULL && lethread_message_count(lethread) != 0) {
		lemessages_load(lethread);
	}

	if (lethread->author->token == NULL) {
		leauthor_load(lethread);
	}

	return lethread;
}

/*
 * Implementation of safe functions required by query.h
 */
status_t s_lethread_save(struct LeThread *lethread) {
	queue_push(lethread_query_queue, lethread, sizeof(struct LeThread));
}

status_t s_lemessages_save(struct LeThread *lethread) {
	queue_push(lemessages_query_queue, lethread, sizeof(struct LeThread));
}

status_t s_lemessage_save(struct LeMessage *lemessage) {
	queue_push(lemessage_query_queue, lemessage, sizeof(struct LeMessage));
}

status_t s_leauthor_save(struct LeThread *lethread) {
	queue_push(leauthor_query_queue, lethread, sizeof(struct LeThread));
}

/*
 * Safe lethread creation required by queue.h
 * 
 * Here we fill lethread_id inddependently on the argument, because we want
 * to keep all the lethreads stay in the right order without collisions.
 */
struct LeThread * s_lethread_create(char *topic, uint64_t lethread_id) {
	struct LeThread    *lethread            = lethread_create(topic, next_lethread_id());


	queue_push(lethread_queue, lethread, sizeof(struct LeThread));

	free(lethread);

	return lethread_queue->last->data; /* Is not very reliable because of multithreading */ 
}

/*
 * Loads already existing lethreads to the queue.
 * 
 * Notice that this fucntion doesn't load lemessages - we load them when we are
 * asked to by client for the first time.
 */
size_t startup() {
	struct LeThread         *lethread;
	uint64_t                 lethread_id;

	DIR                     *srcdir              = opendir(DATA_DIR);
	struct dirent           *dent;
	size_t                   dir_cnt             = 0;


	if (srcdir == NULL) {
		perror("opendir() failed");
		return LESTATUS_CLIB;
	}

	lethread_query_queue = queue_create();
	lemessage_query_queue = queue_create();
	leauthor_query_queue = queue_create();
	leclientinfo_queue = queue_create();
	lethread_queue = queue_create();

	atexit(cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGINT, signal_handler);

	/*
	 * TODO: initialize next_lethread_id_value, next_lethread_id_mutex here
	 */

	lethread = malloc(sizeof(struct LeThread));

    while ((dent = readdir(srcdir)) != NULL) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
			continue;
		}

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			lethread_id = strtoull(dent->d_name, dent->d_name + strlen(dent->d_name), 10);
			if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
				continue;
			}
			leauthor_load(lethread);
			queue_push(lethread_queue, lethread, sizeof(struct LeThread));
			dir_cnt++;
		}
	}

    closedir(srcdir);
	free(lethread);

    return dir_cnt;
}

/* 
 * free()s allocated data
 */
void cleanup() {
	static bool_t       cleaned        = FALSE;

	program_on_finish = TRUE;

	if (!cleaned) {
		cleaned = TRUE;
		queue_delete(leclientinfo_queue, (void (*)(void *))leclientinfo_delete);
		queue_delete(lethread_query_queue, (void (*)(void *))lethread_delete);
		queue_delete(lemessage_query_queue, (void (*)(void *))lethread_delete);
		queue_delete(leauthor_query_queue, (void (*)(void *))lethread_delete);
		queue_delete(lethread_queue, (void (*)(void *))lethread_delete);
	}
}

/* 
 * Clean up and exit if some signal occurs
 */
void signal_handler(const int signum) {
	cleanup();
	exit(signum);
}

uint64_t next_lethread_id() {
	uint64_t            value;


	pthread_mutex_lock(next_lethread_id_mutex);
	value = next_lethread_id_value++;
	pthread_mutex_unlock(next_lethread_id_mutex);

	return value;
}

/*
 * Communicates with a client, gets and sends queries
 * and requests.
 */
void * handle_client(void *arg) {
	struct LeClientInfo     *client_info              = (struct LeClientInfo *)arg;
	struct LeCommandResult   query_result;

	char                     cl_data[PACKET_SIZE];
	char                     sv_data[PACKET_SIZE];

	int64_t                  cl_data_size             = 0;
	int64_t                  sv_data_size             = 0;

	/* =================================== Example ====================================== */

	char client_ip[128];

	uint16_t client_port = ntohs(client_info->addr.sin_port);
	inet_ntop(AF_INET, &(client_info->addr.sin_addr), client_ip, 128);

	sendf(client_info->fd, "Hi! You are fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);
	printf("Connection from fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);

	/* ================================= Example end ==================================== */

	while (!program_on_finish) {
		cl_data_size = recv(client_info->fd, cl_data, PACKET_SIZE, NULL);

		/* Timeout/connection closed */
		if (cl_data_size < 0) {
			break;
		}

		query_result = query_process(cl_data, cl_data_size);

		if (query_result.size == 0) {
			if (query_result.status == LESTATUS_OK) {
				send(client_info->fd, "OK", 2, NULL); /* If query returned nothing, then sends OK */
			}
			else {
				send(client_info->fd, "ERR", 3, NULL); /* Error without description */
			}
		}
		else {
			if (query_result.data != NULL) {
				send(client_info->fd, query_result.data, query_result.size, NULL); /* Sends the query result */
			}
			else {
				send(client_info->fd, "ERR", 3, NULL); /* This case is not valid, sends error without description */
			}
		}

		if (query_result.data != NULL) {
			free(query_result.data);
		}
	}

	close(client_info->fd);

	pthread_exit(0);
}

status_t main(int32_t argc, char *argv[]) {
	struct LeClientInfo     *leclientinfo;

	int32_t                  client_fd;
	int32_t                  server_fd;

	struct sockaddr_in       server_addr;

	struct sockaddr          client_addr;
	socklen_t                client_addr_len;

	socklen_t                socakddr_in_len     = sizeof(struct sockaddr_in);

	pthread_t                client_handler_thread;
	pthread_t                lethread_query_manager_thread;
	pthread_t                lemessage_query_manager_thread;
	pthread_t                leauthor_query_manager_thread;


	startup();

	puts("LeForum Server");

	if (pthread_create(&lethread_query_manager_thread, NULL, lethread_query_manage, NULL) != 0) {
		perror("failed to start lethread query manager");
		return LESTATUS_CLIB;
	}

	if (pthread_create(&lemessage_query_manager_thread, NULL, lemessage_query_manage, NULL) != 0) {
		perror("failed to start lemessage query manager");
		return LESTATUS_CLIB;
	}

	if (pthread_create(&leauthor_query_manager_thread, NULL, leauthor_query_manage, NULL) != 0) {
		perror("failed to start leauthor query manager");
		return LESTATUS_CLIB;
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		perror("socket() failed");
		return LESTATUS_CLIB;
	}

	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		perror("bind() failed");
		return LESTATUS_CLIB;
	}

	if (listen(server_fd, MAX_CONNECTIONS) != 0) {
		perror("listen() failed");
		return LESTATUS_CLIB;
	}

	while (TRUE) {
		client_fd = accept(server_fd, &client_addr, &client_addr_len);

		if (client_fd < 0) {
			perror("accept() failed");
			return LESTATUS_CLIB;
		}

		leclientinfo = malloc(sizeof(struct LeClientInfo));
		leclientinfo->fd = client_fd;

		if (getpeername(client_fd, &leclientinfo->addr, &socakddr_in_len) < 0) {
			perror("getpeername()");
			return LESTATUS_CLIB;
		}

		queue_push(leclientinfo_queue, leclientinfo, sizeof(leclientinfo));

		if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&TIMEOUT, sizeof(TIMEOUT)) < 0) {
			perror("setsockopt() failed");
			return LESTATUS_CLIB;
		}

		if (pthread_create(&client_handler_thread, NULL, handle_client, (void*)leclientinfo) != 0) {
			perror("failed to create client handle");
			return LESTATUS_CLIB;
		}
	}

	return LESTATUS_OK;
}