#include "client/arg.h"

#include <argp.h>

const char *argp_program_version = "LeForum Client v0.1";
const char *argp_program_bug_address = "github.com/sultanowskii";

char doc[] = "LeForum Client - ncurses-based client for LeForum users.";
char args_doc[] = "";

struct argp_option options[] = { 
    { 0 }
};

struct argp le_argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch (key) {
		default: return ARGP_ERR_UNKNOWN;
	}   
return 0;
}