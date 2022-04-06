#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <argp.h>

extern struct argp le_argp;

struct arguments {
	char          *host;
	int32_t        port;
	char          *hello_message;
	struct timeval timeout;
	int32_t        max_connections;
};

/**
 * @brief Argument parser, required by le_argp here (arhp.h) 
 * 
 * @param key Argument key 
 * @param arg Argument value 
 * @param state Context
 * @return Status 
 */
error_t parse_opt(int key, char *arg, struct argp_state *state);