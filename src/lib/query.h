#pragma once

#include <string.h>
#include <inttypes.h>

#include "lib/forum.h"
#include "lib/constants.h"
#include "lib/status.h"

#define CMD_COUNT 2

struct LeCommand {
	char *                     name;
	struct LeCommandResult   (*query_process)(char *, size_t);
};

struct LeCommandResult {
	size_t                     size;
	status_t                   status;
	void *                     data;
};

struct LeCommandResult         cmd_get_lethread(char *raw_data, size_t size);
struct LeCommandResult         cmd_send_lemessage(char *raw_data, size_t size);
struct LeCommandResult         query_parse(char *raw_data, size_t size);