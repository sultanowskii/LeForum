#pragma once

#include <string.h>
#include <inttypes.h>

#include "server/forum.h"

#include "lib/constants.h"
#include "lib/status.h"
#include "lib/queue.h"
#include "lib/shared_ptr.h"

#define CMD_COUNT 5


struct LeCommand {
	char                    *name;
	struct LeCommandResult (*process)(char *, size_t);
};

struct LeCommandResult {
	size_t                   size;
	status_t                 status;
	void                    *data;
};


/* =============================================================================== 
 * 
 * Following functions (in this section) have to implemented 
 * by you as a user of this library. 
 * 
 * The reason is that from this perspective it's unknown how 
 * do you store all the lethreads (in linked list, in array, ...?). 
 */

/**
 * @brief Finds a lethread with the given ID. 
 * The returned LeThread has to be free()'d by you as a user of this interface. 
 * 
 * @param lethread_id ID of LeThread to search for 
 * @return LeThread with the given ID. If not found, LESTATUS_NFND is returned 
 */
SharedPtr *                  lethread_get_by_id(uint64_t lethread_id);

/**
 * @brief Finds a lethread with given parameters, empty data is returned. 
 * 
 * @param topic_part Part of the LeThread topic, based on which the LeThreads will be filtered 
 * @param topic_part_size Size of the topic part 
 * @return Queue containing all the LeThreads that correspond to the conditions 
 */
struct Queue *               lethread_find(char *topic_part, size_t topic_part_size);

/**
 * @brief Multithread-safe lethread_save() function. 
 * The returned LeThread has to be free()'d by you as a user of this interface. 
 * 
 * @param lethread LeThread to save information about 
 * @return LESTATUS_OK on success  
 */
status_t                     s_lethread_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe lemessages_save() function. 
 * 
 * @param lethread Pointer to LeThread message history of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t                     s_lemessages_save(SharedPtr *sharedptr_lemessage);

/**
 * @brief Multithread-safe lemessage_save() function. 
 * 
 * @param lemessage LeMessage to be saved 
 * @return LESTATUS_OK on success 
 */
status_t                     s_lemessage_save(struct LeMessage * lemessage);

/**
 * @brief Multithread-safe leauthor_save() function. 
 * 
 * @param lethread LeThread, author of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t                     s_leauthor_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe lethread_create() function. 
 * 
 * @param topic Topic (aka name) of LeThread 
 * @param lethread_id The ID of LeThread 
 * @return Pointer to the created LeThread 
 */
SharedPtr *                  s_lethread_create(char *topic, uint64_t lethread_id);

/* =============================================================================== */



/**
 * @brief Parses a query, if valid, retrieves LeThread information, including topic, message history, etc. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with data==LeThread object, status==LESATATUS_OK on success. Otherwise corresponding status code is returned the structure 
 */
struct LeCommandResult       cmd_lethread_get(char *raw_data, size_t size);

/**
 * @brief Creates LeThread with given parameters. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success. Otherwise corresponding status code is returned the structure 
 */
struct LeCommandResult       cmd_lethread_create(char *raw_data, size_t size);

/**
 * @brief Parses a query, if valid, finds LeThread by provided parameters. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with data==Query of the filtered LeThreads, status==LESATATUS_OK on success. Otherwise corresponding status code is returned the structure 
 */
struct LeCommandResult       cmd_lethread_find(char *raw_data, size_t size);


/**
 * @brief Parses a query, if valid, adds a new message to the specific lethread. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success. Otherwise corresponding status code is returned the structure 
 */
struct LeCommandResult       cmd_lemessage_create(char *raw_data, size_t size);


/**
 * @brief Assures the connection is not lost.
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success 
 */
struct LeCommandResult       cmd_alive(char *raw_data, size_t size);

/**
 * @brief Tries to find specified command in the data. If command is found, executes its processor. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return Result of the specific query processor that was called if the requested command is valid. Otherwise, returns LeCommandReuslt with status==LESTATUS_ISYN  
 */
struct LeCommandResult       query_process(char *raw_data, size_t size);