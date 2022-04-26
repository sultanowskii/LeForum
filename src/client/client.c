#include "client/client.h"

bool_t             g_working                = TRUE;
bool_t             g_connected              = FALSE;

struct sockaddr_in g_server_addr            = {0};
int                g_server_fd              = nullptr;

LeMeta             g_server_meta            = {0};

bool_t             g_active_thread_exists   = FALSE;
uint64_t           g_active_thread_id       = 0;

struct arguments   arguments;

Queue             *g_server_addr_history    = nullptr;

Queue             *g_server_queries         = nullptr;

pthread_t          g_query_loop_pthread;


const char *MainCmdID_REPR(enum MainCmdIDs id) {
	switch (id) {
		case mcid_SERVER:              return "Server";
		case mcid_THREAD:              return "Thread";
		case mcid_SETTINGS:            return "Settings";
		case mcid_EXIT:                return "Exit";
		default:                       return LESTATUS_NFND;
	}
};

const char *ServerCmdID_REPR(enum ServerCmdIDs id) {
	switch (id) {
		case scid_CONNECT_DISCONNECT:  return g_connected ? "Disconnect" : "Connect";
		case scid_HISTORY:             return "Server history";
		case scid_BACK:                return "Back";
		default:                       return LESTATUS_NFND;
	}
};

const char *ThreadCmdID_REPR(enum ThreadCmdIDs id) {
	switch (id) {
		case tcid_CREATE:              return "Create thread";
		case tcid_FIND:                return "Find thread";
		case tcid_INFO:                return "Thread info";
		case tcid_MESSAGES:            return "Message history";
		case tcid_SEND_MESSAGE:        return "Post message";
		case tcid_BACK:                return "Back";
		default:                       return LESTATUS_NFND;
	}
};

const char *SettingsCmdID_REPR(enum SettingsCmdIDs id) {
	switch (id) {
		case stgcid_BACK:              return "Back";
		default:                       return LESTATUS_NFND;
	}
};

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);
}

/* 
 * You should delete query by yourself after it is complete.
 * Don't delete it until query->completed==TRUE, otherwise it might cause null pointer dereference.
 */
void server_query_add(ServerQuery *query) {
	queue_push(g_server_queries, query, sizeof(ServerQuery));
}

int leclient_loop_process(void (*print_menu)(), void (*print_prefix)()) {
	static char         tmp[128];
	size_t              tmp_size;

	int                 cmd_id;


	print_menu();
	print_prefix();

	if ((int64_t)s_fgets(tmp, 128, stdin) < 0)
		printf("\n");

	cmd_id = atoi(tmp);

	printf("\n");

	return cmd_id;
}

FILE * get_leclient_file(const char *filename, const char *mode, bool_t create) {
	FILE          *file;
	char          *tmp;
	size_t         tmp_size;
	char          *home_dir;
	struct stat    st             = {0};


	home_dir = getenv("HOME");
	tmp_size = strlen(home_dir);
	tmp = malloc(tmp_size + strlen(filename) + 8);
	memset(tmp, 0, tmp_size + strlen(filename) + 8);

	strncpy(tmp, home_dir, tmp_size);
	strcat(tmp, "/");
	strcat(tmp, filename);

	if (stat(tmp, &st) < 0 && !create) {
		free(tmp);
		return LESTATUS_NSFD;
	}

	file = fopen(tmp, mode);

	free(tmp);

	return file;
}

void save_server_addr(const char *addr, uint16_t port) {
	FILE          *file;


	file = get_leclient_file(FILENAME_SERVERS, "a+", TRUE);

	if (!file)
		return;

	fprintf(file, "%s %hd\n", addr, port);

	fclose(file);
}

void load_server_addr_history() {
	FILE          *file;

	char           tmp[64];

	char           addr[32];
	uint16_t       port;

	size_t         bytes_read;

	ServerAddress *tmp_server_addr;


	file = get_leclient_file(FILENAME_SERVERS, "r", FALSE);

	if (file == LESTATUS_NSFD)
		return;

	if (g_server_addr_history != nullptr)
		queue_delete(g_server_addr_history);

	/* ServerAddress structure doesn't require any special clearing */
	g_server_addr_history = queue_create(free);

	while ((int64_t)(bytes_read = s_fgets(tmp, 64, file)) > 0) {
		sscanf(tmp, "%s %hd", addr, &port);

		tmp_server_addr = (ServerAddress *)malloc(sizeof(*tmp_server_addr));

		strncpy(tmp_server_addr->addr, addr, 32);
		tmp_server_addr->port = port;

		queue_push(g_server_addr_history, tmp_server_addr, sizeof(*tmp_server_addr));
    }

	fclose(file);
}

status_t __server_connect(const char *addr, uint16_t port) {
	if (port < 0 || port > 0xffff) {
		puts("Invalid port. Aborted.");
		return LESTATUS_IDAT;
	}

	if ((g_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error occured during socket()");
		return LESTATUS_CLIB;
	}

	g_server_addr.sin_family = AF_INET;
	g_server_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, addr, &g_server_addr.sin_addr) <= 0) {
		puts("Invalid addr is provided. Aborted.");
		return LESTATUS_IDAT;
	}

	if (connect(g_server_fd, (struct sockaddr *)&g_server_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("Error occured during connect()");
		return LESTATUS_CLIB;
	}

	return LESTATUS_OK;
}

inline void print_menu_server() {
	puts("Available server commands:");
	for (enum ServerCmdIDs scid = _scid_BEGIN + 1; scid < _scid_END; scid++)
		printf("%d - %s\n", scid, ServerCmdID_REPR(scid));
}

inline void print_menu_thread() {
	puts("Available thread commands:");
	for (enum ThreadCmdIDs tcid = _tcid_BEGIN + 1; tcid < _tcid_END; tcid++)
		printf("%d - %s\n", tcid, ThreadCmdID_REPR(tcid));
}

inline void print_menu_settings() {
	puts("Available settings commands:");
	for (enum SettingsCmdIDs stgcid = _stgcid_BEGIN + 1; stgcid < _stgcid_END; stgcid++)
		printf("%d - %s\n", stgcid, SettingsCmdID_REPR(stgcid));
}

inline void print_menu_main() {
	puts("Available commands:");
	for (enum MainCmdIDs mcid = _mcid_BEGIN + 1; mcid < _mcid_END; mcid++)
		printf("%d - %s\n", mcid, MainCmdID_REPR(mcid));
}

inline void print_prefix_server() {
	printf("[Server] > ");
}

inline void print_prefix_thread() {
	printf("[Thread] > ");
}

inline void print_prefix_settings() {
	printf("[Settings] > ");
}

inline void print_prefix_main() {
	printf("[*] > ");
}

void cmd_server() {
	enum ServerCmdIDs   cmd_id = _scid_BEGIN;


	while (cmd_id != scid_BACK) {
		cmd_id = leclient_loop_process(print_menu_server, print_prefix_server);

		switch (cmd_id) {
			case scid_CONNECT_DISCONNECT:   g_connected ? cmd_server_disconnect() : cmd_server_connect(); continue;
			case scid_HISTORY:              cmd_server_history(); continue;
			case scid_BACK:                 break;
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
		printf("\n");


	puts("Enter port of the server.");
	print_prefix_server();

	if ((int64_t)s_fgets(tmp_port, sizeof(tmp_port), stdin) < 0)
		printf("\n");

	port = atoi(tmp_port);

	if (__server_connect(tmp_addr, port) != LESTATUS_OK) {
		return;
	}

	save_server_addr(tmp_addr, port);

	g_connected = TRUE;

	tmp_ledata = gen_query_META();
	tmp_query = query_create(parse_response_META, tmp_ledata.data, tmp_ledata.size);
	queue_push(g_server_queries, tmp_query, sizeof(*tmp_query));
	while (tmp_query->completed == FALSE) {

	}
	tmp_lemeta = (LeMeta *)tmp_query->parsed_data;

	query_delete(tmp_query);
	tmp_query = nullptr;

	g_server_meta.max_message_size = tmp_lemeta->max_message_size;
	g_server_meta.min_message_size = tmp_lemeta->min_message_size;
	g_server_meta.max_topic_size = tmp_lemeta->max_topic_size;
	g_server_meta.min_topic_size = tmp_lemeta->min_topic_size;

	free(tmp_lemeta);
	tmp_lemeta = nullptr;
}

void cmd_server_disconnect() {
	close(g_server_fd);
	g_connected = FALSE;
}

void cmd_server_history() {
	QueueNode     *node;
	ServerAddress *server_addr_tmp;
	size_t         i = 1;


	load_server_addr_history();

	node = g_server_addr_history->first;

	while (node != nullptr) {
		server_addr_tmp = (ServerAddress *)node->data;

		printf("%zu. %s:%hd\n", i++, server_addr_tmp->addr, server_addr_tmp->port);

		node = node->next;
	}
}

void cmd_thread() {
	enum ThreadCmdIDs   cmd_id = _tcid_BEGIN;


	if (!g_connected) {
		puts("To see threads, connect to some server first!");
		return;
	}

	while (cmd_id != tcid_BACK) {
		cmd_id = leclient_loop_process(print_menu_thread, print_prefix_thread);

		switch (cmd_id) {
			case tcid_CREATE:               cmd_thread_create(); continue;
			case tcid_FIND:                 cmd_thread_find(); continue;
			case tcid_INFO:                 cmd_thread_info(); continue;
			case tcid_MESSAGES:             cmd_thread_message_history(); continue;
			case tcid_SEND_MESSAGE:         cmd_thread_send_message(); continue;
			case tcid_BACK:                 break;
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


	puts("Thread topic:");
	print_prefix_thread();

	topic = malloc(g_server_meta.max_topic_size + 1);

	if ((topic_size = s_fgets(topic, g_server_meta.max_topic_size, stdin)) < 0) {
		goto THREAD_CREATE_EXIT;
	}

	qdata = gen_query_CTHR(topic, topic_size);
	query = query_create(parse_response_CTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(*query));
	while (query->completed == FALSE) {

	}
	tmp = (CreatedThreadInfo *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	/* TODO: save tmp->token to the file */
	g_active_thread_id = tmp->thread_id;

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
	size_t         search_query_size;
	
	Queue         *found_threads = nullptr;
	QueueNode     *tmp_node = nullptr;
	LeThread      *tmp_thread = nullptr;

	size_t         thread_choice_n;
	char           tmp_buf[64];
	size_t         cntr = 1;

	LeData qdata;
	ServerQuery *query;


	puts("Search query:");
	print_prefix_thread();

	search_query = malloc(g_server_meta.max_topic_size + 1);

	if ((search_query_size = s_fgets(search_query, g_server_meta.max_topic_size, stdin)) < 0) {
		goto THREAD_FIND_EXIT;
	}

	qdata = gen_query_FTHR(search_query, search_query_size);
	query = query_create(parse_response_FTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(*query));
	while (query->completed == FALSE) {

	}
	found_threads = (Queue *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	tmp_node = found_threads->first;

	while (tmp_node != nullptr) {
		tmp_thread = tmp_node->data;

		printf("%zu. %s", cntr, tmp_thread->topic);

		tmp_node = tmp_node->next;
		cntr++;
	}

	if (found_threads->size == 0) {
		puts("Nothing found.");
	}
	else {
		printf("Choose the thread number (1-%zu). Type 'r' any other number if you want to return:\n", found_threads->size);
		print_prefix_thread();

		s_fgets(tmp_buf, sizeof(tmp_buf), stdin);

		thread_choice_n = strtoull(tmp_buf, nullptr, 10);

		if (thread_choice_n < 1 || thread_choice_n > found_threads->size) {
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
	queue_push(g_server_queries, query, sizeof(*query));
	while (query->completed == FALSE) {

	}
	thread = (LeThread *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	printf("Topic: %s\n", thread->topic);
	printf("ID: %zu\n", thread->id);
	printf("Messages posted: %zu\n", lethread_message_count(thread));

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
	queue_push(g_server_queries, query, sizeof(*query));
	while (query->completed == FALSE) {

	}
	thread = (LeThread *)query->parsed_data;
	query_delete(query);
	query = nullptr;

	tmp_node = thread->messages->first;
	
	while (tmp_node != nullptr) {
		tmp_message = (LeMessage *)tmp_node->data;

		printf("msg #%zu", tmp_message->id);
		if (tmp_message->by_lethread_author) 
			printf(" (OP)");
		printf(":\n");
		printf("  %s", tmp_message->text);

		tmp_node = tmp_node->next;
	}

	lethread_delete(thread);
	thread = nullptr;
	tmp_node = nullptr;
	tmp_message = nullptr;
}

void cmd_thread_send_message() {
	char *user_message;
	size_t size;
	LeData qdata;
	ServerQuery *query;
	char *token = nullptr;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	user_message = malloc(g_server_meta.max_message_size + 1);

	printf("Type your message (%zu characters max, no newlines):\n", g_server_meta.max_message_size);

	size = s_fgets(user_message, g_server_meta.max_message_size, stdin);

	/* TODO: load token from file */

	qdata = gen_query_CMSG(g_active_thread_id, user_message, size, token);
	query = query_create(parse_response_GTHR, qdata.data, qdata.size);
	queue_push(g_server_queries, query, sizeof(*query));
	while (query->completed == FALSE) {

	}
	query_delete(query);
	query = nullptr;

	/* TODO: Sanity check */

	free(user_message);
	user_message = nullptr;
}

void cmd_settings() {
	enum SettingsCmdIDs cmd_id = _stgcid_BEGIN;


	while (cmd_id != stgcid_BACK) {
		cmd_id = leclient_loop_process(print_menu_settings, print_prefix_settings);

		switch (cmd_id) {
			case stgcid_BACK:               break;
			default:                        puts("Command not found."); continue;
		}
	}
}

void query_loop() {
	ServerQuery   *tmp_query;
	size_t         part_size;
	size_t         response_size;
	void          *raw_response;


	while (g_working) {
		if (g_connected) {	
			if (g_server_queries->size != 0) {
				tmp_query = (ServerQuery *)queue_pop(g_server_queries);

				/* send packet part by part in order not to exceed send() max size */
				for (size_t i = 0; i < tmp_query->raw_request_data_size / MAX_PACKET_SIZE + 1; i++) {
					part_size = tmp_query->raw_request_data + (MAX_PACKET_SIZE * i);
					send(g_server_fd, part_size, tmp_query->raw_request_data_size, NULL);
				}

				recv(g_server_fd, &response_size, sizeof(response_size), NULL);
				raw_response = malloc(response_size);
				recv(g_server_fd, raw_response, response_size, NULL);
				
				tmp_query->parsed_data = tmp_query->parse_response(raw_response);
				tmp_query->completed = TRUE;

				free(raw_response);
			}
			else {
				send(g_server_fd, "LIVE", 4, NULL);
				
				recv(g_server_fd, &response_size, 8, NULL);
				raw_response = malloc(response_size);
				recv(g_server_fd, raw_response, response_size, NULL);
				/* TODO: Sanity check */
				
				free(response_size);
			}		
		}
	}
}

status_t startup() {
	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	signal(SIGPIPE, SIG_IGN);

	g_server_queries = queue_create(free);

	pthread_create(&g_query_loop_pthread, NULL, query_loop, NULL);
	/* auto-cleanup on termination */
	pthread_detach(g_query_loop_pthread);
}

status_t cleanup() {
	if (g_connected)
		close(g_server_fd);

	if (g_server_queries != nullptr) {
		queue_delete(g_server_queries);
		g_server_queries = nullptr;
	}
		
	return LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	g_working = FALSE;
}

void cmd_exit() {
	cleanup();
	puts("Bye!");
	exit(0);
}

status_t main(size_t argc, char **argv) {
	enum MainCmdIDs     cmd_id;


	load_args(argc, argv);

	startup();

	while (g_working) {
		cmd_id = leclient_loop_process(print_menu_main, print_prefix_main);

		switch (cmd_id) {
			case mcid_SERVER:     cmd_server(); continue;
			case mcid_THREAD:     cmd_thread(); continue;
			case mcid_SETTINGS:   cmd_settings(); continue;
			case mcid_EXIT:       cmd_exit(); continue;
		}
	}

	cleanup();

	return LESTATUS_OK;
}
