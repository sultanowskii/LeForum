#include "client/client.h"

#include <arpa/inet.h>
#include <inttypes.h>
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
int                g_server_fd              = -1;
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
		default:                       return nullptr;
	}
};

inline const char *ServerCmdID_REPR(enum ServerCmdIDs id) {
	switch (id) {
		case SCID_CONNECT_DISCONNECT:  return g_server_connected ? "Disconnect" : "Connect";
		case SCID_INFO:                return "Server information";
		case SCID_HISTORY:             return "Server history";
		case SCID_BACK:                return "Back";
		default:                       return nullptr;
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
		default:                       return nullptr;
	}
};

inline const char *SettingsCmdID_REPR(enum SettingsCmdIDs id) {
	switch (id) {
		case STGCID_BACK:              return "Back";
		default:                       return nullptr;
	}
};

inline void print_menu_server() {
	puts("Available server commands:");
	for (enum ServerCmdIDs scid = _SCID_BEGIN + 1; scid < _SCID_END; scid++)
		printf("%d - %s\n", scid, ServerCmdID_REPR(scid));
}

inline void print_menu_thread() {
	puts("Available thread commands:");
	for (enum ThreadCmdIDs tcid = _TCID_BEGIN + 1; tcid < _TCID_END; tcid++)
		printf("%d - %s\n", tcid, ThreadCmdID_REPR(tcid));
}

inline void print_menu_settings() {
	puts("Available settings commands:");
	for (enum SettingsCmdIDs stgcid = _STGCID_BEGIN + 1; stgcid < _STGCID_END; stgcid++)
		printf("%d - %s\n", stgcid, SettingsCmdID_REPR(stgcid));
}

inline void print_menu_main() {
	puts("Available commands:");
	for (enum MainCmdIDs mcid = _MCID_BEGIN + 1; mcid < _MCID_END; mcid++)
		printf("%d - %s\n", mcid, MainCmdID_REPR(mcid));
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
	ServerQuery *query;
	LeData       ledata;
	LeMeta      *lemeta;

	if (g_server_connected)
		return -LESTATUS_EXST;

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

	ledata = gen_query_META();
	query = query_create(parse_response_META, ledata.data, ledata.size);
	queue_push(g_server_queries, query);
	while (query->completed == FALSE) {

	}
	lemeta = (LeMeta *)query->parsed_data;

	query_delete(query);
	query = nullptr;

	strncpy(g_server_haddr.addr, addr, sizeof(g_server_haddr.addr));
	g_server_haddr.port = port;

	g_server_meta.max_message_size = lemeta->max_message_size;
	g_server_meta.min_message_size = lemeta->min_message_size;
	g_server_meta.max_topic_size = lemeta->max_topic_size;
	g_server_meta.min_topic_size = lemeta->min_topic_size;
	g_server_meta.version = lemeta->version;
	g_server_meta.thread_count = lemeta->thread_count;

	free(lemeta);
	lemeta = nullptr;

	return LESTATUS_OK;
}

status_t __server_disconnect() {
	if (!g_server_connected)
		return -LESTATUS_NFND;

	close(g_server_fd);
	g_server_connected = FALSE;

	g_active_thread_exists = FALSE;
	g_active_thread_id = 0;

	return LESTATUS_OK;
}

int leclient_loop_process(void (*print_menu)(), void (*print_prefix)()) {
	char tmp[64];
	int  cmd_id;

	print_menu();
	print_prefix();

	if ((int64_t)s_fgets(tmp, 128, stdin) < 0)
		newline();

	cmd_id = atoi(tmp);

	newline();

	memset(tmp, 0, sizeof(tmp));

	return cmd_id;
}

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);

	return LESTATUS_OK;
}

/** 
 * You should delete query by yourself after it is complete.
 * Don't delete it until query->completed==TRUE,
 * otherwise it might cause null pointer dereference.
 */
inline void server_query_add(ServerQuery *query) {
	queue_push(g_server_queries, query);
}

status_t get_leclient_file(const char *filename, const char *mode, bool_t create, FILE **file) {
	struct stat  st        = {0};
	char        *filepath;
	size_t       size;

	size = strlen(g_home_dir);
	filepath = calloc(sizeof(char), size + sizeof(DIR_CLIENT) + strlen(filename) + 8);

	sprintf(filepath, "%s/" DIR_CLIENT "%s", g_home_dir, filename);

	if (stat(filepath, &st) < 0 && !create) {
		free(filepath);
		filepath = nullptr;
		return -LESTATUS_NSFD;
	}

	*file = fopen(filepath, mode);

	free(filepath);

	return LESTATUS_OK;
}

status_t server_addr_save(const char *addr, uint16_t port) {
	FILE     *file;
	char     *line                   = nullptr;
	size_t    line_size              = 0;
	char      formatted_address[64];
	bool_t    contains               = FALSE;
	status_t  status;

	memset(formatted_address, 0, sizeof(formatted_address));
	sprintf(formatted_address, "%s %hd", addr, port);

	status = get_leclient_file(FILE_SERVER_HISTORY, "r", FALSE, &file);

	/** 
	 * If the file exists, then we have to check
	 * if it already contains gieven address
	 */
	if (status != -LESTATUS_NSFD) {
		while (getline(&line, &line_size, file) != -1) {
			line[strcspn(line, "\n")] = '\0';
			if (!strcmp(line, formatted_address)) {
				contains = TRUE;
				break;
			}
		}
		fclose(file);
	}

	if (contains)
		return -LESTATUS_EXST;

	if (get_leclient_file(FILE_SERVER_HISTORY, "a+", TRUE, &file) < 0)
		return -LESTATUS_CLIB;

	fputs(formatted_address, file);
	fprintf(file, "\n");

	fclose(file);

	return LESTATUS_OK;
}

status_t server_addr_history_load() {
	FILE     *file;
	HAddress *tmp_server_haddr;
	char      h_addr[32];
	uint16_t  h_port;
	char      tmp[64];
	size_t    bytes_read;

	if (get_leclient_file(FILE_SERVER_HISTORY, "r", FALSE, &file) == -LESTATUS_NSFD)
		return -LESTATUS_NSFD;

	if (g_server_addr_history != nullptr)
		queue_delete(g_server_addr_history);

	/* ServerAddress structure doesn't require any special clearing */
	queue_create(free, &g_server_addr_history);

	while ((int64_t)(bytes_read = s_fgets(tmp, 64, file)) > 0) {
		sscanf(tmp, "%s %hd", h_addr, &h_port);

		tmp_server_haddr = (HAddress *)malloc(sizeof(*tmp_server_haddr));

		strncpy(tmp_server_haddr->addr, h_addr, 32);
		tmp_server_haddr->port = h_port;

		queue_push(g_server_addr_history, tmp_server_haddr);
	}

	fclose(file);

	return LESTATUS_OK;
}

status_t token_save(char *token) {
	FILE *file;
	char  srepr_thread_id[32];

	if (!g_active_thread_exists)
		return -LESTATUS_IDAT;

	memset(srepr_thread_id, 0, sizeof(srepr_thread_id));

	sprintf(srepr_thread_id, "%" PRIu64, g_active_thread_id);
	if (get_leclient_file(srepr_thread_id, "w+", TRUE, &file) == -LESTATUS_CLIB)
		return -LESTATUS_CLIB;

	fprintf(file, "%s", token);

	fclose(file);

	return LESTATUS_OK;
}

status_t token_load(char **token) {
	FILE *file;
	char  srepr_thread_id[32];

	if (!g_active_thread_exists)
		return -LESTATUS_IDAT;

	memset(srepr_thread_id, 0, sizeof(srepr_thread_id));

	snprintf(srepr_thread_id, sizeof(srepr_thread_id), "%" PRIu64, g_active_thread_id);

	if (get_leclient_file(srepr_thread_id, "r", FALSE, &file) == -LESTATUS_NSFD)
		return -LESTATUS_NSFD;

	*token = calloc(sizeof(char), TOKEN_SIZE + 1);

	s_fgets(*token, TOKEN_SIZE + 1, file);

	fclose(file);

	return LESTATUS_OK;
}

void cmd_server() {
	enum ServerCmdIDs cmd_id = _SCID_BEGIN;

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
	char     port[32];
	char     h_addr[32];
	uint16_t h_port;

	puts("Enter addr of the server.");
	print_prefix_server();

	if ((int64_t)s_fgets(h_addr, sizeof(h_addr), stdin) < 0)
		newline();
	newline();

	puts("Enter port of the server.");
	print_prefix_server();

	if ((int64_t)s_fgets(port, sizeof(port), stdin) < 0)
		newline();
	newline();

	h_port = atoi(port);

	if (__server_connect(h_addr, h_port) != LESTATUS_OK) {
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
	printf("Threads on a server: %" PRIu64 "\n", g_server_meta.thread_count);

	newline();
}

void cmd_server_history() {
	QueueNode *node;
	HAddress  *server_haddr;
	char       buf[32];
	size_t     server_choice_n;
	size_t     i                = 1;
	size_t     cntr             = 1;

	if (server_addr_history_load() != LESTATUS_OK) {
		puts("You haven't connected to any server yet.");
		newline();
		return;
	}

	node = g_server_addr_history->first;

	while (node != nullptr) {
		server_haddr = (HAddress *)node->data;

		printf("%zu. %s:%hd\n", i++, server_haddr->addr, server_haddr->port);

		node = node->next;
	}

	newline();

	printf("Choose the server number (1-%zu). Type 'r' or any other number if you don't want to connect to any server:\n", g_server_addr_history->size);
	print_prefix_thread();

	s_fgets(buf, sizeof(buf), stdin);

	server_choice_n = strtoull(buf, nullptr, 10);

	if (server_choice_n < 1 || server_choice_n > g_server_addr_history->size) {
		newline();
		return;
	}
	newline();

	node = g_server_addr_history->first;

	while (cntr < server_choice_n) {
		cntr++;
		node = node->next;
	}

	server_haddr = (HAddress *)node->data;

	__server_disconnect();
	if (__server_connect(server_haddr->addr, server_haddr->port) != LESTATUS_OK) {
		newline();
		return;
	}

	printf("Connected to %s:%hd\n", server_haddr->addr, server_haddr->port);
	newline();
}

void cmd_thread() {
	enum ThreadCmdIDs cmd_id = _TCID_BEGIN;

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
	ServerQuery       *query;
	LeData             qdata;
	CreatedThreadInfo *info;
	char              *topic;
	size_t             topic_size;

	printf("Thread topic (%zu-%zu characters, no newlines):\n", g_server_meta.min_topic_size, g_server_meta.max_topic_size);
	print_prefix_thread();

	topic = malloc(g_server_meta.max_topic_size + 1);

	if ((topic_size = s_fgets_range(topic, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == (size_t)-LESTATUS_IDAT) {
		newline();
		goto THREAD_CREATE_EXIT;
	}
	newline();

	qdata = gen_query_CTHR(topic, topic_size);
	query = query_create(parse_response_CTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query);
	while (query->completed == FALSE) {

	}
	info = (CreatedThreadInfo *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	g_active_thread_exists = TRUE;
	g_active_thread_id = info->thread_id;
	token_save(info->token);

	free(info->token);
	info->token = nullptr;

	free(info);
	info = nullptr;

THREAD_CREATE_EXIT:
	free(topic);
	topic = nullptr;
}

void cmd_thread_find() {
	ServerQuery *query;
	LeData       qdata;
	Queue       *found_threads     = nullptr;
	QueueNode   *node              = nullptr;
	LeThread    *thread            = nullptr;
	char        *search_query      = nullptr;
	size_t       search_query_size = 0;
	size_t       thread_choice_n;
	char         buf[32];
	size_t       cntr              = 1;

	printf("Type part of the topic to find (%zu-%zu characters, no newlines):\n", g_server_meta.min_topic_size, g_server_meta.max_topic_size);
	print_prefix_thread();

	search_query = malloc(g_server_meta.max_topic_size + 1);

	if ((search_query_size = s_fgets_range(search_query, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == (size_t)-LESTATUS_IDAT) {
		newline();
		goto THREAD_FIND_EXIT;
	}

	newline();

	qdata = gen_query_FTHR(search_query, search_query_size);
	query = query_create(parse_response_FTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query);
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

		node = found_threads->first;

		while (node != nullptr) {
			thread = node->data;

			printf("%zu. %s\n", cntr, thread->topic);

			node = node->next;
			cntr++;
		}

		printf("Choose the thread number (1-%zu). Type 'r' or any other number if you want to return:\n", found_threads->size);

		print_prefix_thread();

		s_fgets(buf, sizeof(buf), stdin);

		thread_choice_n = strtoull(buf, nullptr, 10);

		if (thread_choice_n < 1 || thread_choice_n > found_threads->size) {
			newline();
			goto THREAD_FIND_EXIT;
		}

		cntr = 1;

		node = found_threads->first;

		while (node != nullptr && cntr < thread_choice_n) {
			node = node->next;
			cntr++;
		}

		thread = node->data;

		g_active_thread_id = thread->id;
		g_active_thread_exists = TRUE;
	}

	newline();

THREAD_FIND_EXIT:
	queue_delete(found_threads);
	found_threads = nullptr;
	free(search_query);
	search_query = nullptr;
	node = nullptr;
	thread = nullptr;
}

void cmd_thread_info() {
	ServerQuery *query;
	LeData       qdata;
	LeThread    *thread;

	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	qdata = gen_query_GTHR(g_active_thread_id);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query);
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
	ServerQuery *query;
	LeData       qdata;
	LeThread    *thread;
	QueueNode   *node;
	LeMessage   *message;

	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	qdata = gen_query_GTHR(g_active_thread_id);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query);
	while (query->completed == FALSE) {

	}
	thread = (LeThread *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	if (thread->messages->size > 0) {
		node = thread->messages->first;

		while (node != nullptr) {
			message = (LeMessage *)node->data;

			printf("msg #%zu", message->id);
			if (message->by_lethread_author) 
				printf(" (OP)");
			printf(":\n");
			printf("  %s\n", message->text);

			node = node->next;
		}
	}
	else {
		puts("No messages. Be first to post here!");
	}
	newline();

	lethread_delete(thread);
	thread = nullptr;
	node = nullptr;
	message = nullptr;
}

void cmd_thread_send_message() {
	ServerQuery *query;
	LeData       qdata;
	char        *message_text;
	size_t       size          = 0;
	char        *token         = nullptr;
	status_t     status;

	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	message_text = malloc(g_server_meta.max_message_size + 1);

	printf("Type your message (%zu-%zu characters, no newlines):\n", g_server_meta.min_message_size, g_server_meta.max_message_size);

	if ((size = s_fgets_range(message_text, g_server_meta.min_topic_size, g_server_meta.max_topic_size, stdin)) == (size_t)-LESTATUS_IDAT) {
		newline();
		goto THREAD_SEND_MESSAGE_EXIT;
	}
	newline();

	status = token_load(&token);
	if (status == -LESTATUS_IDAT || status == -LESTATUS_NSFD)
		token = nullptr;

	qdata = gen_query_CMSG(g_active_thread_id, message_text, size, token);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query);

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
	free(message_text);
	message_text = nullptr;
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
	ServerQuery *query;
	void        *raw_response;
	size_t       response_size;
	char         buf[32];
	size_t       bytes_read;
	size_t       request_size;

	while (g_working) {
		if (g_server_connected) {
			if (g_server_queries->size != 0) {
				query = (ServerQuery *)queue_pop(g_server_queries);
				request_size = query->raw_request_data_size;

				send(g_server_fd, &request_size, sizeof(request_size), 0);
				if (request_size > MAX_PACKET_SIZE) {
					newline();
					puts("Server acts suspiciously. Disconnecting...");
				}
				s_send(g_server_fd, query->raw_request_data, request_size, 0);

				recv(g_server_fd, &response_size, sizeof(response_size), 0);
				raw_response = calloc(sizeof(char), response_size + 1);
				bytes_read = s_recv(g_server_fd, raw_response, response_size, 0);
				query->parse_response(raw_response, bytes_read, &query->parsed_data);
				query->completed = TRUE;

				free(raw_response);
				raw_response = nullptr;
			}
			else {
				*(size_t *)buf = strlen("LIVE");
				if (send(g_server_fd, buf, sizeof(size_t), 0) < 0) {
					newline();
					puts("Lost connection with the server, your last action might not have been applied.");
					goto LIVE_FAILURE;
				}
				if (send(g_server_fd, "LIVE", *(size_t *)buf, 0) < 0) {
					newline();
					puts("Lost connection with the server, your last action might not have been applied.");
					goto LIVE_FAILURE;
				}

				if (recv(g_server_fd, &response_size, sizeof(response_size), 0) < 0) {
					newline();
					puts("Lost connection with the server, your last action might not have been applied.");
					goto LIVE_FAILURE;
				}
				if (response_size > MAX_PACKET_SIZE) {
					newline();
					puts("Server acts suspiciously. Disconnecting...");
					goto LIVE_FAILURE;

				}
				raw_response = calloc(sizeof(char), response_size + 1);
				if (recv(g_server_fd, raw_response, response_size, 0) < 0) {
					newline();
					puts("Lost connection with the server, your last action might not have been applied.");
					goto LIVE_FAILURE;
				}

				if (strncmp(raw_response, "OK", strlen("OK"))) {
					newline();
					puts("Server acts suspiciously. Disconnecting...");
					goto LIVE_FAILURE;
				}

				free(raw_response);
				raw_response = nullptr;
				continue;
LIVE_FAILURE:
				__server_disconnect();
				continue;
			}
		}
	}
}

status_t startup() {
	struct stat  st         = {0};
	char        *dirpath;

	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	signal(SIGPIPE, SIG_IGN);

	g_home_dir = getenv("HOME");

	dirpath = calloc(sizeof(char), strlen(g_home_dir) + strlen(DIR_CLIENT) + 1);
	strncpy(dirpath, g_home_dir, strlen(g_home_dir));
	le_strcat(dirpath, "/" DIR_CLIENT);

	/* Check if the directory exists, creates if not */
	if (stat(dirpath, &st) == -1)
		mkdir(dirpath, 0700);

	free(dirpath);

	queue_create(free, &g_server_queries);

	pthread_create(&g_query_loop_pthread, NULL, query_loop, NULL);
	/* auto-cleanup on termination */
	pthread_detach(g_query_loop_pthread);

	return LESTATUS_OK;
}

status_t cleanup() {
	if (g_server_connected)
		__server_disconnect();

	if (g_server_queries != nullptr) {
		queue_delete(g_server_queries);
		g_server_queries = nullptr;
	}

	return LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	UNUSED(signum);

	g_working = FALSE;
}

status_t main(int argc, char **argv) {
	enum MainCmdIDs cmd_id;

	load_args(argc, argv);

	startup();

	while (g_working) {
		cmd_id = leclient_loop_process(print_menu_main, print_prefix_main);

		switch (cmd_id) {
			case MCID_SERVER:   cmd_server(); continue;
			case MCID_THREAD:   cmd_thread(); continue;
			case MCID_SETTINGS: cmd_settings(); continue;
			case MCID_EXIT:     cmd_exit(); continue;
			default:            continue;
		}
	}

	cleanup();

	return LESTATUS_OK;
}
