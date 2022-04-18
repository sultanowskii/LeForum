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
		default:                       return LESTATUS_NFND;
	}
};

const char *ThreadCmdID_REPR(enum ThreadCmdIDs id) {
	switch (id) {
		case tcid_INFO:                return "Thread info";
		case tcid_MESSAGES:            return "Message history";
		case tcid_POST_MESSAGE:        return "Post message";
		default:                       return LESTATUS_NFND;
	}
};

const char *SettingsCmdID_REPR(enum SettingsCmdIDs id) {
	switch (id) {
		default:                       return LESTATUS_NFND;
	}
};

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);
}

inline void print_server_menu() {
	for (enum MainCmdIDs scid = _scid_BEGIN + 1; scid < _scid_END; scid++) {
		printf("%d - %s\n", scid, ServerCmdID_REPR(scid));
	}
}

inline void print_thread_menu() {
	for (enum MainCmdIDs tcid = _tcid_BEGIN + 1; tcid < _tcid_END; tcid++) {
		printf("%d - %s\n", tcid, ThreadCmdID_REPR(tcid));
	}
}

inline void print_settings_menu() {
	for (enum MainCmdIDs mcid = _mcid_BEGIN + 1; mcid < _mcid_END; mcid++) {
		printf("%d - %s\n", mcid, MainCmdID_REPR(mcid));
	}
}

inline void print_main_menu() {
	for (enum MainCmdIDs stgcid = _stgcid_BEGIN + 1; stgcid < _stgcid_END; stgcid++) {
		printf("%d - %s\n", stgcid, MainCmdID_REPR(stgcid));
	}
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

status_t main(size_t argc, char **argv) {
	char                tmp[128];
	size_t              tmp_size;

	enum MainCmdIDs     section_id;


	load_args(argc, argv);

	startup();

	while (g_working) {
		print_main_menu();
		printf("[*] > ");
		
		fgets(tmp, 127, stdin);
		
		tmp_size = strlen(tmp);
		if (tmp_size >= 128) {
			tmp_size = 127;
		}
		tmp[tmp_size] = '\0';

		section_id = atoi(tmp);

		printf("%d\n", section_id);

		switch (section_id) {
			// case mcid_SERVER:     cmd_server(); break;
			// case mcid_THREAD:     cmd_thread(); break;
			// case mcid_SETTINGS:   cmd_settings(); break;
			// case mcid_EXIT:       cmd_exit(); break;
			default:                  puts("Unknown command");
		}
	}

	cleanup();

	return LESTATUS_OK;
}
