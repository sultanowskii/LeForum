#pragma once

#include <string.h>
#include <inttypes.h>

#include "lib/forum.h"
#include "lib/constants.h"
#include "lib/status.h"

#define CMD_COUNT 5


/* 
 * ===============================================================================
 * Following functions (in this section) have to implemented
 * by you as a user of this library.
 *
 * The reason is that from this perspective it's unknown how
 * do you store all the lethreads (in linked list, in array, ...?).
 */

/*
 * Finds a lethread with id=lethread_id. If not found, LESTATUS_NFND is returned.
 * 
 * The returned LeThread has to be free()'d by you as a user of this interface
 */
struct LeThread *            lethread_get_by_id(uint64_t lethread_id);


/*
 * Finds a lethread with given parameters, empty data is returned.
 */
struct Queue *               lethread_find(char *topic_part, size_t topic_part_size);

/*
 * Multithread-safe functions.
 * 
 * The returned LeThread (in s_lethread_create()) has to be free()'d by you as a user of this interface
 */
status_t                     s_lethread_save(struct LeThread *lethread);
status_t                     s_lemessages_save(struct LeThread *lethread);
status_t                     s_lemessage_save(struct LeMessage *lemessage);
status_t                     s_leauthor_save(struct LeThread *lethread);

struct LeThread *            s_lethread_create(char *topic, uint64_t lethread_id);

/* 
 * ===============================================================================
 */


struct LeCommand {
	char                    *name;
	struct LeCommandResult (*process)(char *, size_t);
};

struct LeCommandResult {
	size_t                   size;
	status_t                 status;
	void                    *data;
};

struct LeCommandResult       cmd_lethread_get(char *raw_data, size_t size);
struct LeCommandResult       cmd_lethread_create(char *raw_data, size_t size);
struct LeCommandResult       cmd_lemessage_create(char *raw_data, size_t size);
struct LeCommandResult       cmd_lethread_find(char *raw_data, size_t size);
struct LeCommandResult       cmd_alive(char *raw_data, size_t size);

struct LeCommandResult       query_process(char *raw_data, size_t size);