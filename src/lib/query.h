#pragma once

#include <string.h>
#include <inttypes.h>

#include "lib/forum.h"
#include "lib/constants.h"
#include "lib/status.h"

#define CMD_COUNT 4

/*
 * Finds a lethread with id=lethread_id. If not found, LESTATUS_NFND is returned.
 *
 * This function has to implemented by you as a user of this library.
 *
 * The reason is that from this perspective it's unknown how
 * do you store all the lethreads (in linked list, in array, ...?).
 * 
 * By using this interface, you promise you will free() lethread 
 * (the one this function returns) after query processing.
 */
struct LeThread * lethread_get_by_id(uint64_t lethread_id);

/*
 * Multithread-safe functions.
 * 
 * This functions have to implemented by you as a user of this library.
 */
status_t             s_lethread_save(struct LeThread *lethread);
status_t             s_lemessages_save(struct LeThread *lethread);
status_t             s_lemessage_save(struct LeMessage *lemessage);
status_t             s_leauthor_save(struct LeThread *lethread);

struct LeThread *    s_lethread_create(char *topic, uint64_t lethread_id);

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
struct LeCommandResult         cmd_create_lethread(char *raw_data, size_t size);
struct LeCommandResult         cmd_create_lemessage(char *raw_data, size_t size);
struct LeCommandResult         cmd_alive(char *raw_data, size_t size);

struct LeCommandResult         query_process(char *raw_data, size_t size);