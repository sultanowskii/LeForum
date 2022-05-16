#include "server/server.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "server/arg.h"
#include "server/query.h"
#include "lib/communication.h"
#include "lib/constants.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/security.h"
#include "lib/shared_ptr.h"
#include "lib/status.h"
#include "lib/util.h"

struct arguments  arguments;

/* Flag for threads*/
bool_t            g_program_on_finish       = FALSE;

/* Stores the value of the next created LeThread id */
uint64_t          g_next_lethread_id_value  = 0;

/* Pthread mutexes */
pthread_mutex_t   g_next_lethread_id_mutex;

/*
 * Save file query queues with the purpose of prevention data race.
 * SharedPtr is stored here.
 */
Queue            *g_lethread_query_queue;
Queue            *g_lemessages_query_queue;
Queue            *g_lemessage_query_queue;
Queue            *g_leauthor_query_queue;

/* Here we store SharedPtrs to all the LeThreads */
Queue            *g_lethread_queue;

void *lethread_query_manage() {
	SharedPtr *sharedptr_lethread;

	while (!g_program_on_finish) {
		while (!queue_is_empty(g_lethread_query_queue) && !g_program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(g_lethread_query_queue);
			lethread_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}

	return (void *)nullptr;
}

void *lemessages_query_manage() {
	SharedPtr *sharedptr_lethread;

	while (!g_program_on_finish) {
		while (!queue_is_empty(g_lemessages_query_queue) && !g_program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(g_lemessages_query_queue);
			lemessages_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}

	return (void *)nullptr;
}

void *lemessage_query_manage() {
	while (!g_program_on_finish) {
		while (!queue_is_empty(g_lemessage_query_queue) && !g_program_on_finish) {
			lemessage_save(queue_pop(g_lemessage_query_queue));
		}
	}

	return (void *)nullptr;
}

void *leauthor_query_manage() {
	SharedPtr *sharedptr_lethread;

	while (!g_program_on_finish) {
		while (!queue_is_empty(g_leauthor_query_queue) && !g_program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(g_leauthor_query_queue);
			leauthor_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}

	return (void *)nullptr;
}

status_t leclientinfo_delete(LeClientInfo *clinfo) {
	NULLPTR_PREVENT(clinfo, -LESTATUS_NPTR)

	free(clinfo);
	clinfo = nullptr;

	return LESTATUS_OK;
}

SharedPtr *lethread_get_by_id(uint64_t lethread_id) {
	LeThread  *lethread       = nullptr;
	LeThread  *lethread_found = nullptr;
	QueueNode *node           = g_lethread_queue->first;

	while (node != NULL) {
		lethread = (LeThread *)((SharedPtr *)node->data)->data;
		if (lethread->id == lethread_id) {
			lethread_found = lethread;
			break;
		}
		node = node->next;
	}

	NULLPTR_PREVENT(lethread_found, -LESTATUS_NFND)

	if (lethread_found->messages->first == nullptr && lethread_message_count(lethread_found) != 0)
		lemessages_load(lethread_found);
	if (lethread_found->author == nullptr || lethread_found->author->token == nullptr)
		leauthor_load(lethread_found);

	return sharedptr_add(node->data);
}

Queue *lethread_find(char *topic_part) {
	LeThread  *lethread;
	QueueNode *node             = g_lethread_queue->first;
	Queue     *lethreads_match;

	NULLPTR_PREVENT(topic_part, -LESTATUS_NPTR)

	queue_create(sharedptr_delete, &lethreads_match);

	while (node != NULL) {
		lethread = ((SharedPtr *)node->data)->data;
		if (strstr(lethread->topic, topic_part) != NULL)
			queue_push(lethreads_match, sharedptr_add(node->data));
		node = node->next;
	}

	return lethreads_match;
}

inline status_t s_lethread_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, -LESTATUS_NPTR)

	queue_push(g_lethread_query_queue, sharedptr_add(sharedptr_lethread));
	return LESTATUS_OK;
}

inline status_t s_lemessages_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, -LESTATUS_NPTR)

	queue_push(g_lemessages_query_queue, sharedptr_add(sharedptr_lethread));
	return LESTATUS_OK;
}

inline status_t s_lemessage_save(LeMessage *lemessage) {
	NULLPTR_PREVENT(lemessage, -LESTATUS_NPTR)

	queue_push(g_lemessage_query_queue, lemessage);
	return LESTATUS_OK;
}

inline status_t s_leauthor_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, -LESTATUS_NPTR)

	queue_push(g_leauthor_query_queue, sharedptr_add(sharedptr_lethread));
	return LESTATUS_OK;
}

SharedPtr *s_lethread_create(char *topic, uint64_t lethread_id) {
	SharedPtr *sharedptr_lethread;
	LeThread  *tmp;

	UNUSED(lethread_id);

	/** 
	 * Here we fill lethread_id independently on the argument, 
	 * because we want to keep all the lethreads stay in the right order without collisions. 
	 */
	lethread_create(topic, next_lethread_id(), &tmp);
	sharedptr_create(tmp, lethread_delete, &sharedptr_lethread);

	queue_push(g_lethread_queue, sharedptr_lethread);

	return sharedptr_add(sharedptr_lethread); 
}

inline size_t get_lethread_count() {
	return g_lethread_queue->size;
}

inline const char *get_version() {
	return argp_program_version;
}

inline void lemeta_load() {
	FILE        *metafile;
	struct stat  st                  = {0};

	if (stat(DIR_SERVER "/" FILE_LEMETA, &st) == -1) {
		g_next_lethread_id_value = 0;
	}
	else {
		metafile = fopen(DIR_SERVER "/" FILE_LEMETA, "rb");
		fread(&g_next_lethread_id_value, sizeof(g_next_lethread_id_value), 1, metafile);
		fclose(metafile);
	}
}

inline void lemeta_save() {
	FILE *metafile;

	metafile = fopen(DIR_SERVER "/" FILE_LEMETA, "wb");
	fwrite(&g_next_lethread_id_value, sizeof(g_next_lethread_id_value), 1, metafile);
	fclose(metafile);
}

size_t startup() {
	LeThread      *lethread;
	uint64_t       lethread_id;
	DIR           *srcdir;
	struct dirent *dent;
	size_t         dir_cnt      = 0;
	struct stat    st           = {0};
	SharedPtr     *sptr;

	/* Check if the directory exists, creates if not */
	if (stat(DIR_SERVER, &st) == -1)
		mkdir(DIR_SERVER, 0700);

	srcdir = opendir(DIR_SERVER);
	if (srcdir == NULL) {
		perror("opendir() failed");
		return -LESTATUS_CLIB;
	}

	queue_create(sharedptr_delete, &g_lethread_query_queue);
	queue_create(sharedptr_delete, &g_lemessages_query_queue);
	queue_create(lemessage_delete, &g_lemessage_query_queue);
	queue_create(sharedptr_delete, &g_leauthor_query_queue);
	queue_create(sharedptr_delete, &g_lethread_queue);

	atexit(cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGINT, signal_handler);

	/* Prevents process termination on SIGPIPE*/
	signal(SIGPIPE, SIG_IGN);

	while ((dent = readdir(srcdir)) != NULL) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			continue;

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0)
			continue;

		if (S_ISDIR(st.st_mode)) {
			lethread = (LeThread *)malloc(sizeof(*lethread));

			lethread_id = strtoull(dent->d_name, NULL, 10);

			if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
				if (lethread != nullptr) {
					free(lethread);
					lethread = nullptr;
				}
				continue;
			}

			leauthor_load(lethread);

			sharedptr_create(lethread, lethread_delete, &sptr);
			queue_push(g_lethread_queue, sptr);

			dir_cnt++;
		}
	}

	closedir(srcdir);

	lemeta_load();

	pthread_mutex_init(&g_next_lethread_id_mutex, NULL);

	return dir_cnt;
}

void cleanup() {
	static bool_t  cleaned   = FALSE;

	g_program_on_finish = TRUE;

	if (!cleaned) {
		cleaned = TRUE;

		lemeta_save();

		queue_delete(g_lethread_query_queue);
		g_lethread_query_queue = nullptr;
		queue_delete(g_lemessages_query_queue);
		g_lemessages_query_queue = nullptr;
		queue_delete(g_lemessage_query_queue);
		g_lemessage_query_queue = nullptr;
		queue_delete(g_leauthor_query_queue);
		g_leauthor_query_queue = nullptr;
		queue_delete(g_lethread_queue);
		g_lethread_queue = nullptr;

		pthread_mutex_destroy(&g_next_lethread_id_mutex);
	}
}

void signal_handler(const int signum) {
	cleanup();
	exit(signum);
}

uint64_t next_lethread_id() {
	uint64_t value;

	pthread_mutex_lock(&g_next_lethread_id_mutex);
	value = g_next_lethread_id_value++;
	/* Not really sure if it is necessary or not */
	lemeta_save();
	pthread_mutex_unlock(&g_next_lethread_id_mutex);

	return value;
}

void *handle_client(void *arg) {
	LeClientInfo    *client_info;
	LeCommandResult  query_result;
	char            *cl_data;
	size_t           cl_expected_data_size    = 0;
	size_t           cl_data_size             = 0;
	char             tmp[64];
	const char      *lestatus_representation;

	NULLPTR_PREVENT(arg, -LESTATUS_NPTR)

	cl_data = malloc(MAX_PACKET_SIZE + 1);
	client_info = (LeClientInfo *)arg;

	while (!g_program_on_finish) {
		cl_expected_data_size = 0;
		recv(client_info->fd, &cl_expected_data_size, sizeof(cl_expected_data_size), 0);
		/* We simply break the connection with client if it requests enormous size */
		if (cl_expected_data_size > MAX_PACKET_SIZE) {
			puts("go duck :)");
			break;
		}
		cl_data_size = s_recv(client_info->fd, cl_data, cl_expected_data_size, 0);

		/* Timeout/connection closed */
		if (cl_data_size <= 0) {
			query_result.data = NULL;
			break;
		}

		cl_data[cl_data_size] = '\0';
		query_result = query_process(cl_data, cl_data_size);
		memset(cl_data, 0, cl_data_size + 1);

		if (query_result.size == 0) {
			/* Status without description */
			lestatus_representation = get_lestatus_string_repr(query_result.status);
			*(size_t *)tmp = strlen(lestatus_representation);
			send(client_info->fd, &tmp, sizeof(size_t), 0);
			send(client_info->fd, lestatus_representation, *(size_t *)tmp, 0);
		}
		else {
			if (query_result.data != NULL) {
				/* Sends the query result */
				send(client_info->fd, &query_result.size, sizeof(size_t), 0);
				s_send(client_info->fd, query_result.data, query_result.size, 0);
			}
			else {
				/* Unexpected, sends error without description */
				*(size_t *)tmp = strlen("ERR");
				send(client_info->fd, tmp, sizeof(size_t), 0);
				send(client_info->fd, "ERR", strlen("ERR"), 0);
			}
		}

		if (query_result.data != NULL) {
			free(query_result.data);
			query_result.data = nullptr;
		}
	}

	close(client_info->fd);

	free(cl_data);
	cl_data = nullptr;

	leclientinfo_delete(client_info);
	client_info = nullptr;

	pthread_exit(0);
}

status_t main(int argc, char *argv[]) {
	LeClientInfo       *leclientinfo;
	int                 client_fd;
	int                 server_fd;
	struct sockaddr_in  server_addr;
	struct sockaddr     client_addr;
	socklen_t           client_addr_len;
	socklen_t           socakddr_in_len                 = sizeof(struct sockaddr_in);
	pthread_t           client_handler_thread;
	pthread_t           lethread_query_manager_thread;
	pthread_t           lemessage_query_manager_thread;
	pthread_t           leauthor_query_manager_thread;

	arguments.host = "0.0.0.0";
	arguments.port = 7431;
	arguments.timeout.tv_sec = 3;
	arguments.timeout.tv_usec = 0;
	arguments.max_connections = 100;
	arguments.hello_message = "Have a great day!";

	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);

	if (arguments.port < 0 || arguments.port > 0xffff) {
		puts("invalid port provided");
		return -LESTATUS_IDAT;
	}

	if (arguments.max_connections <= 0) {
		puts("invalid connection limit");
		return -LESTATUS_IDAT;
	}

	startup();

	if (arguments.hello_message != nullptr && strlen(arguments.hello_message) > 0)
		puts(arguments.hello_message);

	if (pthread_create(&lethread_query_manager_thread, NULL, lethread_query_manage, NULL) != 0) {
		perror("failed to start lethread query manager");
		return -LESTATUS_CLIB;
	}
	pthread_detach(lethread_query_manager_thread);

	if (pthread_create(&lemessage_query_manager_thread, NULL, lemessage_query_manage, NULL) != 0) {
		perror("failed to start lemessage query manager");
		return -LESTATUS_CLIB;
	}
	pthread_detach(lemessage_query_manager_thread);

	if (pthread_create(&leauthor_query_manager_thread, NULL, leauthor_query_manage, NULL) != 0) {
		perror("failed to start leauthor query manager");
		return -LESTATUS_CLIB;
	}
	pthread_detach(leauthor_query_manager_thread);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) {
		perror("socket() failed");
		return -LESTATUS_CLIB;
	}

	server_addr.sin_addr.s_addr = inet_addr(arguments.host);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(arguments.port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		perror("bind() failed");
		return -LESTATUS_CLIB;
	}

	if (listen(server_fd, arguments.max_connections) != 0) {
		perror("listen() failed");
		return -LESTATUS_CLIB;
	}

	printf("Server is listening on %s:%d\n", arguments.host, arguments.port);

	while (TRUE) {
		client_fd = accept(server_fd, &client_addr, &client_addr_len);

		if (client_fd < 0) {
			perror("accept() failed");
			return -LESTATUS_CLIB;
		}

		leclientinfo = malloc(sizeof(*leclientinfo));
		leclientinfo->fd = client_fd;

		if (getpeername(client_fd, &leclientinfo->addr, &socakddr_in_len) < 0) {
			perror("getpeername()");
			return -LESTATUS_CLIB;
		}

		if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&arguments.timeout, sizeof(arguments.timeout)) < 0) {
			perror("setsockopt() failed");
			return -LESTATUS_CLIB;
		}

		if (pthread_create(&client_handler_thread, NULL, handle_client, (void *)leclientinfo) != 0) {
			perror("failed to create client handle");
			return -LESTATUS_CLIB;
		}
		pthread_detach(client_handler_thread);
	}

	return LESTATUS_OK;
}
