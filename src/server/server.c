#include "server/server.h"


struct arguments arguments;

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
pthread_mutex_t    next_lethread_id_mutex;

/*
 * Save file query queues with the purpose of prevention data race. SharedPtr is stored here.
 */
struct Queue      *lethread_query_queue;
struct Queue      *lemessages_query_queue;
struct Queue      *lemessage_query_queue;
struct Queue      *leauthor_query_queue;

/*
 * Here we store SharedPtrs to all the LeThreads
 */
struct Queue      *lethread_queue;

void * lethread_query_manage() {
	SharedPtr               *sharedptr_lethread;

	while (!program_on_finish) {
		while (!queue_is_empty(lethread_query_queue) && !program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(lethread_query_queue);
			lethread_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}
}

void * lemessages_query_manage() {
	SharedPtr               *sharedptr_lethread;

	while (!program_on_finish) {
		while (!queue_is_empty(lemessages_query_queue) && !program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(lemessages_query_queue);
			lemessages_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}
}

void * lemessage_query_manage() {
	struct LeMessage        *lemessage;

	while (!program_on_finish) {
		while (!queue_is_empty(lemessage_query_queue) && !program_on_finish) {
			lemessage_save(queue_pop(lemessage_query_queue));
		}
	}
}

void * leauthor_query_manage() {
	SharedPtr               *sharedptr_lethread;

	while (!program_on_finish) {
		while (!queue_is_empty(leauthor_query_queue) && !program_on_finish) {
			sharedptr_lethread = (SharedPtr *)queue_pop(leauthor_query_queue);
			leauthor_save(sharedptr_lethread->data);
			sharedptr_delete(sharedptr_lethread);
			sharedptr_lethread = nullptr;
		}
	}
}

status_t leclientinfo_delete(struct LeClientInfo *clinfo) {
	NULLPTR_PREVENT(clinfo, LESTATUS_NPTR)

	free(clinfo);
	clinfo = nullptr;
}

SharedPtr * lethread_get_by_id(uint64_t lethread_id) {
	struct LeThread         *lethread       = nullptr;
	struct LeThread         *lethread_found = nullptr;
	struct QueueNode        *node           = lethread_queue->first;

	while (node != NULL) {
		lethread = (struct LeThread *)((SharedPtr *)node->data)->data;
		if (lethread->id == lethread_id) {
			lethread_found = lethread;
			break;
		}
		node = node->next;
	}

	NULLPTR_PREVENT(lethread_found, LESTATUS_NFND)

	if (lethread_found->messages->first == nullptr && lethread_message_count(lethread_found) != 0) {
		lemessages_load(lethread_found);
	}
	if (lethread_found->author->token == nullptr) {
		leauthor_load(lethread_found);
	}

	return sharedptr_add(node->data);
}

struct Queue * lethread_find(char *topic_part, size_t topic_part_size) {
	struct LeThread         *lethread;
	struct QueueNode        *node           = lethread_queue->first;

	struct Queue            *lethreads_match;


	NULLPTR_PREVENT(topic_part, LESTATUS_NPTR)

	lethreads_match = queue_create(sharedptr_delete);

	while (node != NULL) {
		lethread = ((SharedPtr *)node->data)->data;
		if (strstr(lethread->topic, topic_part) != NULL) {
			queue_push(lethreads_match, sharedptr_add(node->data), sizeof(SharedPtr));
		}
		node = node->next;
	}

	return lethreads_match;
}

status_t s_lethread_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, LESTATUS_NPTR)

	queue_push(lethread_query_queue, sharedptr_add(sharedptr_lethread), sizeof(SharedPtr));
	return LESTATUS_OK;
}

status_t s_lemessages_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, LESTATUS_NPTR)

	queue_push(lemessages_query_queue, sharedptr_add(sharedptr_lethread), sizeof(SharedPtr));
	return LESTATUS_OK;
}

status_t s_lemessage_save(struct LeMessage *lemessage) {
	NULLPTR_PREVENT(lemessage, LESTATUS_NPTR)

	queue_push(lemessage_query_queue, lemessage, sizeof(struct LeMessage));
	return LESTATUS_OK;
}

status_t s_leauthor_save(SharedPtr *sharedptr_lethread) {
	NULLPTR_PREVENT(sharedptr_lethread, LESTATUS_NPTR)

	queue_push(leauthor_query_queue, sharedptr_add(sharedptr_lethread), sizeof(SharedPtr));
	return LESTATUS_OK;
}

SharedPtr * s_lethread_create(char *topic, uint64_t lethread_id) {
	NULLPTR_PREVENT(topic, LESTATUS_NPTR)

	/* Here we fill lethread_id independently on the argument, 
	 * because we want to keep all the lethreads stay in the right order without collisions. 
	 */
	SharedPtr               *sharedptr_lethread  = sharedptr_create(lethread_create(topic, next_lethread_id()), lethread_delete);

	queue_push(lethread_queue, sharedptr_lethread, sizeof(SharedPtr));

	return sharedptr_add(sharedptr_lethread); 
}

void lemeta_load() {
	FILE                    *metafile;
	struct stat              st                  = {0};


	if (stat(DATA_DIR "/" FILENAME_LEMETA, &st) == -1) {
		next_lethread_id_value = 0;
	}
	else {
		metafile = fopen(DATA_DIR "/" FILENAME_LEMETA, "rb");
		fread(&next_lethread_id_value, sizeof(next_lethread_id_value), 1, metafile);
		fclose(metafile);
	}
}

void lemeta_save() {
	FILE                    *metafile;

	metafile = fopen(DATA_DIR "/" FILENAME_LEMETA, "wb");
	fwrite(&next_lethread_id_value, sizeof(next_lethread_id_value), 1, metafile);
	fclose(metafile);
}

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

	lethread_query_queue = queue_create(sharedptr_delete);
	lemessages_query_queue = queue_create(sharedptr_delete);
	lemessage_query_queue = queue_create(lemessage_delete);
	leauthor_query_queue = queue_create(sharedptr_delete);
	lethread_queue = queue_create(sharedptr_delete);

	atexit(cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGINT, signal_handler);

	/* Prevents process termination on SIGPIPE*/
	signal(SIGPIPE, SIG_IGN);

	while ((dent = readdir(srcdir)) != NULL) {
		struct stat st;

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
			continue;
		}

		if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			lethread = (struct LeThread *)malloc(sizeof(struct LeThread));

			lethread_id = strtoull(dent->d_name, dent->d_name + strlen(dent->d_name), 10);

			if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
				if (lethread != nullptr) {
					free(lethread);
					lethread = nullptr;
				}
				continue;
			}

			leauthor_load(lethread);

			queue_push(lethread_queue, sharedptr_create(lethread, lethread_delete), sizeof(SharedPtr));

			dir_cnt++;
		}
	}

	closedir(srcdir);

	lemeta_load();

	pthread_mutex_init(&next_lethread_id_mutex, NULL);

	return dir_cnt;
}

void cleanup() {
	static bool_t       cleaned        = FALSE;

	FILE                    *metafile;


	program_on_finish = TRUE;

	if (!cleaned) {
		cleaned = TRUE;

		lemeta_save();

		queue_delete(lethread_query_queue);
		lethread_query_queue = nullptr;
		queue_delete(lemessages_query_queue);
		lemessages_query_queue = nullptr;
		queue_delete(lemessage_query_queue);
		lemessage_query_queue = nullptr;
		queue_delete(leauthor_query_queue);
		leauthor_query_queue = nullptr;
		queue_delete(lethread_queue);
		lethread_queue = nullptr;

		pthread_mutex_destroy(&next_lethread_id_mutex);
	}
}

void signal_handler(const int signum) {
	cleanup();
	exit(signum);
}

uint64_t next_lethread_id() {
	uint64_t            value;


	pthread_mutex_lock(&next_lethread_id_mutex);
	value = next_lethread_id_value++;
	lemeta_save(); /* Not really sure if it is necessary or not */
	pthread_mutex_unlock(&next_lethread_id_mutex);

	return value;
}

void * handle_client(void *arg) {
	struct LeClientInfo     *client_info;
	struct LeCommandResult   query_result;

	char                    *cl_data;

	size_t                   cl_expected_data_size    = 0;
	size_t                   cl_data_size             = 0;

	char                     tmp[64];


	NULLPTR_PREVENT(arg, LESTATUS_NPTR)

	cl_data = malloc(MAX_PACKET_SIZE);
	client_info = (struct LeClientInfo *)arg;

	/* =================================== Example ====================================== */

	char client_ip[128];

	uint16_t client_port = ntohs(client_info->addr.sin_port);
	inet_ntop(AF_INET, &(client_info->addr.sin_addr), client_ip, 128);

	sendf(client_info->fd, "Hi! You are fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);
	printf("Connection from fd=%d, addr=%s:%hu\n", client_info->fd, client_ip, client_port);

	/* ================================= Example end ==================================== */

	while (!program_on_finish) {
		recv(client_info->fd, &cl_expected_data_size, sizeof(cl_expected_data_size), NULL);
		cl_data_size = recv(client_info->fd, cl_data, MIN(cl_expected_data_size, MAX_PACKET_SIZE - 1), NULL);

		/* Timeout/connection closed */
		if (cl_data_size <= 0) {
			break;
		}

		cl_data[cl_data_size] = '\0';
		query_result = query_process(cl_data, cl_data_size);
		memset(cl_data, 0, MIN(cl_data_size + 1, MAX_PACKET_SIZE));

		if (query_result.size == 0) {
			if (query_result.status == LESTATUS_OK) {
				*(size_t *)tmp = 2;
				send(client_info->fd, tmp, sizeof(size_t), NULL);
				send(client_info->fd, "OK", 2, NULL); /* If query returned nothing, then sends OK */
			}
			else {
				*(size_t *)tmp = 3;
				send(client_info->fd, tmp, sizeof(size_t), NULL);
				send(client_info->fd, "ERR", 3, NULL); /* Error without description */
			}
		}
		else {
			if (query_result.data != NULL) {
				send(client_info->fd, &query_result.size, sizeof(size_t), NULL);
				send(client_info->fd, query_result.data, query_result.size, NULL); /* Sends the query result */
			}
			else {
				*(size_t *)tmp = 3;
				send(client_info->fd, tmp, sizeof(size_t), NULL);
				send(client_info->fd, "ERR", 3, NULL); /* This case is not valid, sends error without description */
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
	struct LeClientInfo     *leclientinfo;

	int                      client_fd;
	int                      server_fd;

	struct sockaddr_in       server_addr;

	struct sockaddr          client_addr;
	socklen_t                client_addr_len;

	socklen_t                socakddr_in_len     = sizeof(struct sockaddr_in);

	pthread_t                client_handler_thread;
	pthread_t                lethread_query_manager_thread;
	pthread_t                lemessage_query_manager_thread;
	pthread_t                leauthor_query_manager_thread;

	arguments.host = "0.0.0.0";
	arguments.port = 7431;
	arguments.timeout.tv_sec = 3;
	arguments.timeout.tv_usec = 0;
	arguments.max_connections = 100;
	arguments.hello_message = "Have a great day!";

	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);

	if (arguments.port < 0 || arguments.port > 0xffff) {
		puts("invalid port provided");
		return LESTATUS_IDAT;
	}

	if (arguments.max_connections <= 0) {
		puts("invalid connection limit");
		return LESTATUS_IDAT;
	}

	startup();

	if (arguments.hello_message != nullptr && strlen(arguments.hello_message) > 0) {
		puts(arguments.hello_message);
	}

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

	server_addr.sin_addr.s_addr = inet_addr(arguments.host);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(arguments.port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		perror("bind() failed");
		return LESTATUS_CLIB;
	}

	if (listen(server_fd, arguments.max_connections) != 0) {
		perror("listen() failed");
		return LESTATUS_CLIB;
	}

	printf("Server is listening on %s:%d\n", arguments.host, arguments.port);

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

		if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&arguments.timeout, sizeof(arguments.timeout)) < 0) {
			perror("setsockopt() failed");
			return LESTATUS_CLIB;
		}

		if (pthread_create(&client_handler_thread, NULL, handle_client, (void *)leclientinfo) != 0) {
			perror("failed to create client handle");
			return LESTATUS_CLIB;
		}
	}

	return LESTATUS_OK;
}