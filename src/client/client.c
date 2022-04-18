#include "client/client.h"

bool_t             g_working             = TRUE;

struct arguments   arguments;

const char *SectionID_REPR[] = {
	[mcid_SERVER]       = "Server",
	[mcid_THREAD]       = "Thread",
	[mcid_SETTINGS]     = "Settings",
	[mcid_EXIT]         = "Exit",
};

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);
}

inline void print_main_menu() {
	for (enum MainCmdIDs mcid = _mcid_BEGIN + 1; mcid < _mcid_END; mcid++) {
		printf("%d - %s\n", mcid, SectionID_REPR[mcid]);
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
			// case mcid_SERVER:      section_server(); break;
			// case mcid_THREAD:      section_thread(); break;
			// case mcid_SETTINGS:    section_settings(); break;
			// case mcid_EXIT:        section_exit(); break;
			default:                  puts("Unknown command");
		}
	}

	cleanup();

	return LESTATUS_OK;
}
