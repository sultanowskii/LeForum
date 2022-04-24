#include "client/client.h"

bool_t             g_working                = TRUE;
bool_t             g_connected              = FALSE;

struct sockaddr_in g_server_addr            = {0};
int                g_server_fd              = nullptr;

size_t             g_min_message_size       = 0;
size_t             g_max_message_size       = 0;
size_t             g_min_topic_size         = 0;
size_t             g_max_topic_size         = 0;

bool_t             g_active_thread_exists   = FALSE;
uint64_t           g_active_thread_id       = 0;

struct arguments   arguments;

Queue*             g_server_addr_history    = nullptr;


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

status_t startup() {
	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	signal(SIGPIPE, SIG_IGN);
}

status_t cleanup() {
	if (g_connected)
		close(g_server_fd);

	return LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	g_working = FALSE;
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

	/* TODO: create pthread and run conversation there */

	/* TODO: Send META query and fill g_max_*, g_min_* values */
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
			case tcid_FIND:                 cmd_thread_find(); continue;
			case tcid_INFO:                 cmd_thread_info(); continue;
			case tcid_MESSAGES:             cmd_thread_message_history(); continue;
			case tcid_SEND_MESSAGE:         cmd_thread_send_message(); continue;
			case tcid_BACK:                 break;
			default:                        puts("Command not found."); continue;
		}
	}
}

void cmd_thread_find() {
	char          *search_query = nullptr;
	
	Queue         *found_threads = nullptr;
	QueueNode     *tmp_node = nullptr;
	LeThread      *tmp_thread = nullptr;

	size_t         thread_choice_n;
	char           tmp_buf[64];
	size_t         cntr = 1;


	puts("Search query:");
	print_prefix_thread();

	search_query = malloc(g_max_topic_size + 1);

	if ((int64_t)s_fgets(search_query, g_max_topic_size, stdin) < 0) {
		goto THREAD_FIND_EXIT;
	}

	/* TODO: Send FTHR query */

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
	LeThread      *tmp_thread;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	/* TODO: Update information using GTHR query */

	printf("Topic: %s\n", tmp_thread->topic);
	printf("ID: %zu\n", tmp_thread->id);
	printf("Messages posted: %zu\n", lethread_message_count(tmp_thread));

	lethread_delete(tmp_thread);
	tmp_thread = nullptr;
}

void cmd_thread_message_history() {
	LeThread      *tmp_thread;
	QueueNode     *tmp_node;
	LeMessage     *tmp_message;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	/* TODO: update information using GTHR query */

	tmp_node = tmp_thread->messages->first;
	
	while (tmp_node != nullptr) {
		tmp_message = (LeMessage *)tmp_node->data;

		printf("msg #%zu", tmp_message->id);
		if (tmp_message->by_lethread_author) 
			printf(" (OP)");
		printf(":\n");
		printf("  %s", tmp_message->text);

		tmp_node = tmp_node->next;
	}

	lethread_delete(tmp_thread);
	tmp_thread = nullptr;
	tmp_node = nullptr;
	tmp_message = nullptr;
}

void cmd_thread_send_message() {
	char          *user_message;


	if (!g_active_thread_exists) {
		puts("To use this command, choose some thread first!");
		return;
	}

	user_message = malloc(g_max_message_size + 1);

	printf("Type your message (%zu characters max, no newlines):\n", g_max_message_size);

	s_fgets(user_message, g_max_message_size, stdin);

	/* TODO: Send CMSG query */

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

void cmd_exit() {
	cleanup();
	puts("Bye!");
	exit(0);
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
