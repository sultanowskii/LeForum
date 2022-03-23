#pragma once

#include <string.h>
#include <inttypes.h>

#include "lib/forum.h"
#include "lib/constants.h"
#include "lib/status.h"

#define CMD_COUNT 2

/*
 * Finds a lethread with id=lethread_id. If not found, LESTATUS_NFND is returned.
 *
 * This function has to be implemented somewhere.
 *
 * The concept is that from this perspective it's unknown how
 * do you store all the lethreads (in linked list, in array, ...?).
 * Therefore, as a user of this interface, you have to implement
 * this function yourself.
 */
struct LeThread * lethread_get_by_id(uint64_t lethread_id);

struct LeCommand {
	char *                     name;
	struct LeCommandResult   (*process)(char *, size_t);
};

struct LeCommandResult {
	size_t                     size;
	status_t                   status;
	void *                     data;
};

struct LeCommandResult         cmd_get_lethread(char *raw_data, size_t size);
struct LeCommandResult         cmd_send_lemessage(char *raw_data, size_t size);
struct LeCommandResult         query_process(char *raw_data, size_t size);