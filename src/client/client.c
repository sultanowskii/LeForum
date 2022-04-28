#include "client/client.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client/arg.h"
#include "client/query.h"
#include "lib/communication.h"
#include "lib/constants.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/util.h"

bool_t             g_working                = TRUE;
struct arguments   arguments;

bool_t             g_server_connected       = FALSE;
struct sockaddr_in g_server_addr            = {0};
int                g_server_fd              = nullptr;
LeMeta             g_server_meta            = {0};
Queue             *g_server_addr_history    = nullptr;
HAddress           g_server_haddr           = {0};

Queue             *g_server_queries         = nullptr;

bool_t             g_active_thread_exists   = FALSE;
uint64_t           g_active_thread_id       = 0;

pthread_t          g_query_loop_pthread     = 0;

char              *g_home_dir               = nullptr;


inline const char *MainCmdID_REPR(enum MainCmdIDs id) {
	switch (id) {
		case MCID_SERVER:              return "Server";
		case MCID_THREAD:              return "Thread";
		case MCID_SETTINGS:            return "Settings";
		case MCID_EXIT:                return "Exit";
		default:                       return -LESTATUS_NFND;
	}
};

inline const char *ServerCmdID_REPR(enum ServerCmdIDs id) {
	switch (id) {
		case SCID_CONNECT_DISCONNECT:  return g_server_connected ? "Disconnect" : "Connect";
		case SCID_INFO:                return "Server information";
		case SCID_HISTORY:             return "Server history";
		case SCID_BACK:                return "Back";
		default:                       return -LESTATUS_NFND;
	}
};

inline const char *ThreadCmdID_REPR(enum ThreadCmdIDs id) {
	switch (id) {
		case TCID_CREATE:              return "Create thread";
		case TCID_FIND:                return "Find thread";
		case TCID_INFO:                return "Thread info";
		case TCID_MESSAGES:            return "Message history";
		case TCID_SEND_MESSAGE:        return "Post message";
		case TCID_BACK:                return "Back";
		default:                       return -LESTATUS_NFND;
	}
};

inline const char *SettingsCmdID_REPR(enum SettingsCmdIDs id) {
	switch (id) {
		case STGCID_BACK:              return "Back";
		default:                       return -LESTATUS_NFND;
	}
};

inline void print_menu_server() {
	puts("Available server commands:");
	for (enum ServerCmdIDs SCID = _SCID_BEGIN + 1; SCID < _SCID_END; SCID++)
		printf("%d - %s\n", SCID, ServerCmdID_REPR(SCID));
}

inline void print_menu_thread() {
	puts("Available thread commands:");
	for (enum ThreadCmdIDs TCID = _TCID_BEGIN + 1; TCID < _TCID_END; TCID++)
		printf("%d - %s\n", TCID, ThreadCmdID_REPR(TCID));
}

inline void print_menu_settings() {
	puts("Available settings commands:");
	for (enum SettingsCmdIDs STGCID = _STGCID_BEGIN + 1; STGCID < _STGCID_END; STGCID++)
		printf("%d - %s\n", STGCID, SettingsCmdID_REPR(STGCID));
}

inline void print_menu_main() {
	puts("Available commands:");
	for (enum MainCmdIDs MCID = _MCID_BEGIN + 1; MCID < _MCID_END; MCID++)
		printf("%d - %s\n", MCID, MainCmdID_REPR(MCID));
}

inline void print_prefix_server() {
	newline();
	printf("[Server] > ");
}

inline void print_prefix_thread() {
	newline();
	printf("[Thread] > ");
}

inline void print_prefix_settings() {
	newline();
	printf("[Settings] > ");
}

inline void print_prefix_main() {
	newline();
	printf("[*] > ");
}

status_t __server_connect(const char *addr, uint16_t port) {
	LeMeta         *tmp_lemeta;
	LeData          tmp_ledata;
	ServerQuery    *tmp_query;


	if (g_server_connected)
		return -LESTATUS_EXST;

	if (port < 0 || port > 0xffff) {
		puts("Invalid port. Aborted.");
		return -LESTATUS_IDAT;
	}

	if ((g_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error occured during socket()");
		return -LESTATUS_CLIB;
	}

	g_server_addr.sin_family = AF_INET;
	g_server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, addr, &g_server_addr.sin_addr) <= 0) {
		puts("Invalid addr is provided. Aborted.");
		return -LESTATUS_IDAT;
	}

	if (connect(g_server_fd, (struct sockaddr *)&g_server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("Error occured during connect()");
		return -LESTATUS_CLIB;
	}

	server_addr_save(addr, port);

	g_server_connected = TRUE;

	tmp_ledata = gen_query_META();
	tmp_query = query_create(parse_response_META, tmp_ledata.data, tmp_ledata.size);
	queue_push(g_server_queries, tmp_query, sizeof(tmp_query));
	while (tmp_query->completed == FALSE) {

	}
	tmp_lemeta = (LeMeta *)tmp_query->parsed_data;

	query_delete(tmp_query);
	tmp_query = nullptr;

	strncpy(g_server_haddr.addr, addr, sizeof(g_server_haddr.addr));
	g_server_haddr.port = port;

	g_server_meta.max_message_size = tmp_lemeta->max_message_size;
	g_server_meta.min_message_size = tmp_lemeta->min_message_size;
	g_server_meta.max_topic_size = tmp_lemeta->max_topic_size;
	g_server_meta.min_topic_size = tmp_lemeta->min_topic_size;
	g_server_meta.version = tmp_lemeta->version;
	g_server_meta.thread_count = tmp_lemeta->thread_count;

	free(tmp_lemeta);
	tmp_lemeta = nullptr;

	return -LESTATUS_OK;
}

status_t __server_disconnect() {
	if (!g_server_connected)
		return -LESTATUS_NFND;

	close(g_server_fd);
	g_server_connected = FALSE;

	g_active_thread_exists = FALSE;
	g_active_thread_id = 0;

	return -LESTATUS_OK;
}

int leclient_loop_process(void (*print_menu)(), void (*print_prefix)()) {
	static char         tmp[128];
	size_t              tmp_size;

	int                 cmd_id;

	print_menu();
	print_prefix();

	if ((int64_t)s_fgets(tmp, 128, stdin) < 0)
		newline();

	cmd_id = atoi(tmp);

	newline();

	memset(tmp, 0, 128);

	return cmd_id;
}

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);
}

/* 
 * You should delete query by yourself after it is complete.
 * Don't delete it until query->completed==TRUE, otherwise it might cause null pointer dereference.
 */
inline void server_query_add(ServerQuery *query) {
	queue_push(g_server_queries, query, sizeof(ServerQuery));
}

FILE * get_leclient_file(const char *filename, const char *mode, bool_t create) {
	FILE          *file;
	char          *tmp;
	size_t         tmp_size;
	struct stat    st             = {0};


	tmp_size = strlen(g_home_dir);
	tmp = calloc(sizeof(char), tmp_size + strlen(filename) + 8);

	strncpy(tmp, g_home_dir, tmp_size);
	strcat(tmp, "/" DIR_CLIENT);
	strcat(tmp, filename);

	if (stat(tmp, &st) < 0 && !create) {
		free(tmp);
		return -LESTATUS_NSFD;
	}

	file = fopen(tmp, mode);

	free(tmp);

	return file;
}

status_t server_addr_save(const char *addr, uint16_t port) {
	FILE          *file;
	char           formatted_address[64];
	bool_t         contains = FALSE;
	char          *line = nullptr;
	size_t         line_size = 0;


	memset(formatted_address, 0, sizeof(formatted_address));
	sprintf(formatted_address, "%s %hd", addr, port);

	file = get_leclient_file(FILE_SERVER_HISTORY, "r", FALSE);

	/* if file exists, then we have to check if it already contains gieven address */
	if (file != -LESTATUS_NSFD) {
		while (getline(&line, &line_size, file) != -1) {
			line[strcspn(line, "\n")] = '\0';
			if (!strcmp(line, formatted_address)) {
				contains = TRUE;
				break;
			}
		}
		fclose(file);
	}

	if (contains) {
		return -LESTATUS_EXST;
	}

	file = get_leclient_file(FILE_SERVER_HISTORY, "a+", TRUE);

	if (file < 0)
		return -LESTATUS_CLIB;

	fputs(formatted_address, file);
	fprintf(file, "\n");

	fclose(file);

	return -LESTATUS_OK;
}

status_t server_addr_history_load() {
	FILE          *file;

	char           tmp[64];

	char           addr[32];
	uint16_t       port;

	size_t         bytes_read;

	HAddress      *tmp_server_haddr;


	file = get_leclient_file(FILE_SERVER_HISTORY, "r", FALSE);

	if (file == -LESTATUS_NSFD)
		return -LESTATUS_NSFD;

	if (g_server_addr_history != nullptr)
		queue_delete(g_server_addr_history);

	/* ServerAddress structure doesn't require any special clearing */
	g_server_addr_history = queue_create(free);

	while ((int64_t)(bytes_read = s_fgets(tmp, 64, file)) > 0) {
		sscanf(tmp, "%s %hd", addr, &port);

		tmp_server_haddr = (HAddress *)malloc(sizeof(*tmp_server_haddr));

		strncpy(tmp_server_haddr->addr, addr, 32);
		tmp_server_haddr->port = port;

		queue_push(g_server_addr_history, tmp_server_haddr, sizeof(tmp_server_haddr));
	}

	fclose(file);

	return -LESTATUS_OK;
}

status_t token_save(char *token) {
	FILE      *file;
	char       thread_id_repr[32];


	if (!g_active_thread_exists) {
		return -LESTATUS_IDAT;
	}

	memset(thread_id_repr, 0, sizeof(thread_id_repr));

	sprintf(thread_id_repr, "%llu", g_active_thread_id);
	file = get_leclient_file(thread_id_repr, "w+", TRUE);

	fprintf(file, "%s", token);

	fclose(file);
}

char * token_load() {
	FILE *file;
	char thread_id_repr[32];
	char *token;


	if (!g_active_thread_exists) {
		return -LESTATUS_IDAT;
	}

	memset(thread_id_repr, 0, sizeof(thread_id_repr));

	snprintf(thread_id_repr, sizeof(thread_id_repr), "%llu", g_active_thread_id);
	file = get_leclient_file(thread_id_repr, "r", FALSE);

	if (file == -LESTATUS_NSFD) {
		return -LESTATUS_NSFD;
	}

	token = calloc(sizeof(char), TOKEN_SIZE + 1);

	s_fgets(token, TOKEN_SIZE + 1, file);

	fclose(file);

	return token;
}

void cmd_server() {
	enum ServerCmdIDs   cmd_id = _SCID_BEGIN;


	while (cmd_id != SCID_BACK) {
		cmd_id = leclient_loop_process(print_menu_server, print_prefix_server);

		switch (cmd_id) {
			case SCID_CONNECT_DISCONNECT:   g_server_connected ? cmd_server_disconnect() : cmd_server_connect(); continue;
			case SCID_HISTORY:              cmd_server_history(); continue;
			case SCID_INFO:                 cmd_server_info(); break;
			case SCID_BACK:                 break;
			default:                        puts("Command not found."); continue;
		}
	}
}

void cmd_server_connect() {
	char                tmp_port[32];
	char                tmp_addr[32];
	size_t              tmp_size;
	uint16_t            port;
	ServerQuery        *tmp_query;
	LeMeta             *tmp_lemeta;
	LeData              tmp_ledata;


	puts("Enter addr of the server.");
	print_prefix_server();

	if ((int64_t)s_fgets(tmp_addr, sizeof(tmp_addr), stdin) < 0)
		newline();
	newline();

	puts("Enter port of the server.");
	print_prefix_server();

	if ((int64_t)s_fgets(tmp_port, sizeof(tmp_port), stdin) < 0)
		newline();
	newline();

	port = atoi(tmp_port);

	if (__server_connect(tmp_addr, port) != -LESTATUS_OK) {
		newline();
		return;
	}
	
	puts("Connected.");
	newline();
}

void cmd_server_disconnect() {
	puts("Disconnected.");
	newline();
	__server_disconnect();
}

void cmd_server_info() {
	if (!g_server_connected) {
		puts("Can't get any information - you are not connected to any server.");		
		newline();
		return;
	}

	printf("Server address: %s:%hd\n", g_server_haddr.addr, g_server_haddr.port);
	printf("Server version: %s\n", g_server_meta.version);
	printf("Threads on a server: %llu\n", g_server_meta.thread_count);

	newline();
}

void cmd_server_history() {
	QueueNode     *node;
	HAddress      *tmp_server_haddr;
	size_t         i = 1;
	char           tmp_buf[32];
	size_t         server_choice_n;
	size_t         cntr;


	if (server_addr_history_load() != LESTATUS_OK) {
		puts("You haven't connected to any server yet.");
		newline();
		return;
	}

	node = g_server_addr_history->first;

	while (node != nullptr) {
		tmp_server_haddr = (HAddress *)node->data;

		printf("%zu. %s:%hd\n", i++, tmp_server_haddr->addr, tmp_server_haddr->port);

		node = node->next;
	}

	newline();

	printf("Choose the server number (1-%zu). Type 'r' or any other number if you don't want to connect to any server:\n", g_server_addr_history->size);
	print_prefix_thread();

	s_fgets(tmp_buf, sizeof(tmp_buf), stdin);

	server_choice_n = strtoull(tmp_buf, nullptr, 10);

	if (server_choice_n < 1 || server_choice_n > g_server_addr_history->size) {
		newline();
		return;
	}
	newline();

	node = g_server_addr_history->first;
	cntr = 1;

	while (cntr < server_choice_n) {
		cntr++;
		node = node->next;
	}

	tmp_server_haddr = (HAddress *)node->data;

	__server_disconnect();
	if (__server_connect(tmp_server_haddr->addr, tmp_server_haddr->port) != -LESTATUS_OK) {
		newline();
		return;
	}

	printf("Connected to %s:%hd\n", tmp_server_haddr->addr, tmp_server_haddr->port);
	newline();
}

void cmd_thread() {
	enum ThreadCmdIDs   cmd_id = _TCID_BEGIN;


	if (!g_server_connected) {
		puts("To see threads, connect to some server first!");
		newline();
		return;
	}

	while (cmd_id != TCID_BACK) {
		cmd_id = leclient_loop_process(print_menu_thread, print_prefix_thread);

		switch (cmd_id) {
			case TCID_CREATE:               cmd_thread_create(); continue;
			case TCID_FIND:                 cmd_thread_find(); continue;
			case TCID_INFO:                 cmd_thread_info(); continue;
			case TCID_MESSAGES:             cmd_thread_message_history(); continue;
			case TCID_SEND_MESSAGE:         cmd_thread_send_message(); continue;
			case TCID_BACK:                 break;
			default:                        puts("Command not found."); continue;
		}
	}
}

void cmd_thread_create() {
	char *topic;
	size_t topic_size;
	LeData qdata;
	ServerQuery *query;
	CreatedThreadInfo *tmp;
	size_t tmp_n;


	printf("Thread topic (%zu-%zu characters, no newlines):\n", g_server_meta.min_topic_size, g_server_meta.max_topic_size);
	print_prefix_thread();

	topic = malloc(g_server_meta.max_topic_size + 1);

	if ((topic_size = s_fgets_range(topic, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == -LESTATUS_IDAT) {
		newline();
		goto THREAD_CREATE_EXIT;
	}
	newline();

	qdata = gen_query_CTHR(topic, topic_size);
	query = query_create(parse_response_CTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(query));
	while (query->completed == FALSE) {

	}
	tmp = (CreatedThreadInfo *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	g_active_thread_exists = TRUE;
	g_active_thread_id = tmp->thread_id;
	token_save(tmp->token);

	free(tmp->token);
	tmp->token = nullptr;
	free(tmp);
	tmp = nullptr;

THREAD_CREATE_EXIT:
	free(topic);
	topic = nullptr;
}

void cmd_thread_find() {
	char          *search_query = nullptr;
	size_t         search_query_size = 0;
	
	Queue         *found_threads = nullptr;
	QueueNode     *tmp_node = nullptr;
	LeThread      *tmp_thread = nullptr;

	size_t         thread_choice_n;
	char           tmp_buf[64];
	size_t         cntr = 1;
	size_t         tmp = 0;

	LeData         qdata;
	ServerQuery   *query;


	printf("Type part of the topic to find (%zu-%zu characters, no newlines):\n", g_server_meta.min_topic_size, g_server_meta.max_topic_size);
	print_prefix_thread();

	search_query = malloc(g_server_meta.max_topic_size + 1);

	if ((search_query_size = s_fgets_range(search_query, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == -LESTATUS_IDAT) {
		newline();
		goto THREAD_FIND_EXIT;
	}

	newline();

	qdata = gen_query_FTHR(search_query, search_query_size);
	query = query_create(parse_response_FTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(query));
	while (query->completed == FALSE) {

	}
	found_threads = (Queue *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	if (found_threads->size == 0) {
		puts("Nothing found.");
	}
	else {
		printf("Here are the threads that match '%s':\n", search_query);
		
		tmp_node = found_threads->first;

		while (tmp_node != nullptr) {
			tmp_thread = tmp_node->data;

			printf("%zu. %s\n", cntr, tmp_thread->topic);

			tmp_node = tmp_node->next;
			cntr++;
		}

		printf("Choose the thread number (1-%zu). Type 'r' or any other number if you want to return:\n", found_threads->size);
		
		print_prefix_thread();

		s_fgets(tmp_buf, sizeof(tmp_buf), stdin);

		thread_choice_n = strtoull(tmp_buf, nullptr, 10);

		if (thread_choice_n < 1 || thread_choice_n > found_threads->size) {
			newline();
			goto THREAD_FIND_EXIT;
		}

		cntr = 1;

		tmp_node = found_threads->first;

		while (tmp_node != nullptr && cntr < thread_choice_n) {
			tmp_node = tmp_node->next;
			cntr++;
		}

		tmp_thread = tmp_node->data;

		g_active_thread_id = tmp_thread->id;
		g_active_thread_exists = TRUE;
	}

	newline();

THREAD_FIND_EXIT:
	queue_delete(found_threads);
	found_threads = nullptr;
	free(search_query);
	search_query = nullptr;
	tmp_node = nullptr;
	tmp_thread = nullptr;
}

void cmd_thread_info() {
	LeThread *thread;
	LeData qdata;
	ServerQuery *query;

	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	qdata = gen_query_GTHR(g_active_thread_id);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(query));
	while (query->completed == FALSE) {

	}
	thread = (LeThread *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	printf("Topic: %s\n", thread->topic);
	printf("ID: %zu\n", thread->id);
	printf("Messages posted: %zu\n", lethread_message_count(thread));
	newline();

	lethread_delete(thread);
	thread = nullptr;
}

void cmd_thread_message_history() {
	LeThread *thread;
	QueueNode *tmp_node;
	LeMessage *tmp_message;
	LeData qdata;
	ServerQuery *query;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	qdata = gen_query_GTHR(g_active_thread_id);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(query));
	while (query->completed == FALSE) {

	}
	thread = (LeThread *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	if (thread->messages->size > 0) {
		tmp_node = thread->messages->first;
		
		while (tmp_node != nullptr) {
			tmp_message = (LeMessage *)tmp_node->data;

			printf("msg #%zu", tmp_message->id);
			if (tmp_message->by_lethread_author) 
				printf(" (OP)");
			printf(":\n");
			printf("  %s\n", tmp_message->text);

			tmp_node = tmp_node->next;
		}
	}
	else {
		puts("No messages. Be first to post here!");
	}
	newline();

	lethread_delete(thread);
	thread = nullptr;
	tmp_node = nullptr;
	tmp_message = nullptr;
}

void cmd_thread_send_message() {
	char *user_message;
	size_t size = 0;
	LeData qdata;
	ServerQuery *query;
	char *token = nullptr;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	user_message = malloc(g_server_meta.max_message_size + 1);

	printf("Type your message (%zu-%zu characters, no newlines):\n", g_server_meta.min_message_size, g_server_meta.max_message_size);
	
	if ((size = s_fgets_range(user_message, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == -LESTATUS_IDAT) {
		newline();
		goto THREAD_SEND_MESSAGE_EXIT;
	}
	newline();

	token = token_load();
	if (token == -LESTATUS_IDAT || token == -LESTATUS_NSFD)
		token = nullptr;

	qdata = gen_query_CMSG(g_active_thread_id, user_message, size, token);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(query));

	while (query->completed == FALSE) {

	}

	/* TODO: Sanity check */

	query_delete(query);
	query = nullptr;

	if (token != nullptr) {
		free(token);
		token = nullptr;
	}

THREAD_SEND_MESSAGE_EXIT:
	free(user_message);
	user_message = nullptr;
}

void cmd_settings() {
	enum SettingsCmdIDs cmd_id = _STGCID_BEGIN;


	while (cmd_id != STGCID_BACK) {
		cmd_id = leclient_loop_process(print_menu_settings, print_prefix_settings);

		switch (cmd_id) {
			case STGCID_BACK:               break;
			default:                        puts("Command not found."); continue;
		}
	}
}

void cmd_exit() {
	cleanup();
	puts("Bye!");
	exit(0);
}

void query_loop() {
	ServerQuery   *tmp_query;
	size_t         part_size;
	size_t         response_size;
	void          *raw_response;
	size_t         bytes_read;
	size_t         tmp_n;
	char           tmp[32];


	while (g_working) {
		if (g_server_connected) {	
			if (g_server_queries->size != 0) {
				tmp_query = (ServerQuery *)queue_pop(g_server_queries);
				tmp_n = tmp_query->raw_request_data_size;

				send(g_server_fd, &tmp_n, sizeof(size_t), NULL);
				send(g_server_fd, tmp_query->raw_request_data, tmp_n, NULL);
				
				recv(g_server_fd, &response_size, sizeof(response_size), NULL);

				raw_response = calloc(sizeof(char), response_size + 1);
				bytes_read = recv(g_server_fd, raw_response, response_size, NULL);
				
				tmp_query->parsed_data = tmp_query->parse_response(raw_response, bytes_read);
				tmp_query->completed = TRUE;

				free(raw_response);
			}
			else {
				*(size_t *)tmp = strlen("LIVE");
				send(g_server_fd, tmp, sizeof(size_t), NULL);
				send(g_server_fd, "LIVE", tmp, NULL);
				
				recv(g_server_fd, &response_size, sizeof(size_t), NULL);
				raw_response = calloc(sizeof(char), response_size + 1);
				recv(g_server_fd, raw_response, response_size, NULL);
				/* TODO: Sanity check */
				
				free(raw_response);
			}		
		}
	}
}

status_t startup() {
	struct stat         st                  = {0};
	char               *tmp;


	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	signal(SIGPIPE, SIG_IGN);
	
	g_home_dir = getenv("HOME");

	tmp = calloc(sizeof(char), strlen(g_home_dir) + strlen(DIR_CLIENT) + 1);
	strncpy(tmp, g_home_dir, strlen(g_home_dir));
	strcat(tmp, "/" DIR_CLIENT);

	/* Check if the directory exists, creates if not */
	if (stat(tmp, &st) == -1) {
		mkdir(tmp, 0700);
	}

	free(tmp);

	g_server_queries = queue_create(free);

	pthread_create(&g_query_loop_pthread, NULL, query_loop, NULL);
	/* auto-cleanup on termination */
	pthread_detach(g_query_loop_pthread);
}

status_t cleanup() {
	if (g_server_connected)
		__server_disconnect();

	if (g_server_queries != nullptr) {
		queue_delete(g_server_queries);
		g_server_queries = nullptr;
	}
		
	return -LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	g_working = FALSE;
}

status_t main(size_t argc, char **argv) {
	enum MainCmdIDs     cmd_id;


	load_args(argc, argv);

	startup();

	while (g_working) {
		cmd_id = leclient_loop_process(print_menu_main, print_prefix_main);

		switch (cmd_id) {
			case MCID_SERVER:     cmd_server(); continue;
			case MCID_THREAD:     cmd_thread(); continue;
			case MCID_SETTINGS:   cmd_settings(); continue;
			case MCID_EXIT:       cmd_exit(); continue;
		}
	}

	cleanup();

	return -LESTATUS_OK;
}
