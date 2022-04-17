#include "client/client.h"

bool_t             g_working             = TRUE;

struct arguments arguments;

status_t load_args(int argc, char **argv) {
	argp_parse(&le_argp, argc, argv, 0, 0, &arguments);
}

status_t startup() {
	signal(SIGTERM, stop_program_handle);
	signal(SIGINT, stop_program_handle);

	/* Prevents process termination on SIGPIPE */
	signal(SIGPIPE, SIG_IGN);
}

status_t cleanup() {
	return LESTATUS_OK;
}

void stop_program_handle(const int signum) {
	g_working = FALSE;
}

status_t main(size_t argc, char **argv) {
	int                 ch;


	load_args(argc, argv);

	startup();

	while (g_working) {
	}

	cleanup();

	return LESTATUS_OK;
}
