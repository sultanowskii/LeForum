#include "server/arg.h"

#include <argp.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

const char  *argp_program_version     = "LeForum Server v0.1";
const char  *argp_program_bug_address = "github.com/sultanowskii";
char         doc[]                    = "LeForum Server - multithread forum server that handles structures (such as threads, messages) and provides an API via TCP.";
char         args_doc[]               = "";

struct argp_option options[] = { 
	{"host", 'h', "ADDR", 0, "Host address to bind server on"},
	{"port", 'p', "NUM", 0, "Port to bind server on"},
	{"hello-message", 'm', "TEXT", 0, "Hello message on startup"},
	{"timeout", 't', "NUM", 0, "Timeout for client communication"},
	{"max-connections", 'c', "NUM", 0, "Max number of clients able to connect at one time. This argument is passed to listen(2), therefore, on some platforms, the actual limit may vary"},
	{ 0 }
};

struct argp le_argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
		case 'h': arguments->host = arg; break;
		case 'p': arguments->port = atoi(arg); break;
		case 'm': arguments->hello_message = arg; break;
		case 't': arguments->timeout.tv_sec = atoi(arg); break;
		case 'c': arguments->max_connections = atoi(arg); break;
		default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}