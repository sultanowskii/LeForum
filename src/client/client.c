#include "client/client.h"

bool_t             g_working             = TRUE;
bool_t             g_connected           = FALSE;

struct arguments   arguments;

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
		case scid_CONNECT_DISCONNECT:  return g_connected ? "Connect" : "Disconnect";
		case scid_HISTORY:             return "Server history";
		case scid_BACK:                return "Back";
		default:                       return LESTATUS_NFND;
	}
};

const char *ThreadCmdID_REPR(enum ThreadCmdIDs id) {
	switch (id) {
		case tcid_INFO:                return "Thread info";
		case tcid_MESSAGES:            return "Message history";
		case tcid_POST_MESSAGE:        return "Post message";
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

inline void print_menu_server() {
	for (enum ServerCmdIDs scid = _scid_BEGIN + 1; scid < _scid_END; scid++) {
		printf("%d - %s\n", scid, ServerCmdID_REPR(scid));
	}
}

inline void print_menu_thread() {
	for (enum ThreadCmdIDs tcid = _tcid_BEGIN + 1; tcid < _tcid_END; tcid++) {
		printf("%d - %s\n", tcid, ThreadCmdID_REPR(tcid));
	}
}

inline void print_menu_settings() {
	for (enum SettingsCmdIDs stgcid = _stgcid_BEGIN + 1; stgcid < _stgcid_END; stgcid++) {
		printf("%d - %s\n", stgcid, SettingsCmdID_REPR(stgcid));
	}
}

inline void print_menu_main() {
	for (enum MainCmdIDs mcid = _mcid_BEGIN + 1; mcid < _mcid_END; mcid++) {
		printf("%d - %s\n", mcid, MainCmdID_REPR(mcid));
	}
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
}

status_t cleanup() {
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
			case scid_CONNECT_DISCONNECT:   return;
			case scid_HISTORY:              return;
			case scid_BACK:                 return;
			default:                        puts("Command not found");
		}
	}
}

void cmd_thread() {
	enum ThreadCmdIDs   cmd_id = _tcid_BEGIN;


	while (cmd_id != tcid_BACK) {
		cmd_id = leclient_loop_process(print_menu_thread, print_prefix_thread);

		switch (cmd_id) {
			case tcid_INFO:                 return;
			case tcid_MESSAGES:             return;
			case tcid_POST_MESSAGE:         return;
			case tcid_BACK:                 return;
			default:                        puts("Command not found");
		}
	}
}

void cmd_settings() {
	enum SettingsCmdIDs cmd_id = _stgcid_BEGIN;


	while (cmd_id != stgcid_BACK) {
		cmd_id = leclient_loop_process(print_menu_settings, print_prefix_settings);

		switch (cmd_id) {
			case stgcid_BACK:               return;
			default:                        puts("Command not found");
		}
	}
	
}

void cmd_exit() {
	cleanup();
	exit(0);	
}

int leclient_loop_process(void (*print_menu)(), void (*print_prefix)()) {
	static char         tmp[128];
	size_t              tmp_size;

	int                 cmd_id;


	print_menu();
	print_prefix();
	
	fgets(tmp, 127, stdin);
	
	tmp_size = strlen(tmp);
	if (tmp_size >= 128) {
		tmp_size = 127;
	}
	tmp[tmp_size] = '\0';

	cmd_id = atoi(tmp);

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
