#ifndef LEFORUM_SERVER_QUERY_H_
#define LEFORUM_SERVER_QUERY_H_

#include <stdlib.h>

#include "lib/forum.h"
#include "lib/shared_ptr.h"
#include "lib/status.h"
#include "lib/queue.h"

#define CMD_COUNT 6

struct LeCommand {
	char                    *name;
	struct LeCommandResult (*process)(char *, size_t);
};
typedef struct LeCommand LeCommand;

struct LeCommandResult {
	void     *data;
	size_t    size;
	status_t  status;
};
typedef struct LeCommandResult LeCommandResult;

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
SharedPtr *lethread_get_by_id(uint64_t lethread_id);

/**
 * @brief Finds a lethread with given parameters, empty data is returned. 
 * 
 * @param topic_part Part of the LeThread topic, 
 * based on which the LeThreads will be filtered 
 * @return Queue containing all the LeThreads that 
 * correspond to the conditions 
 */
Queue *lethread_find(char *topic_part);

/**
 * @brief Multithread-safe lethread_save() function. 
 * The returned LeThread has to be free()'d by you as a user of this interface. 
 * 
 * @param lethread LeThread to save information about 
 * @return LESTATUS_OK on success  
 */
status_t s_lethread_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe lemessages_save() function. 
 * 
 * @param lethread Pointer to LeThread message history of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_lemessages_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe lemessage_save() function. 
 * 
 * @param lemessage LeMessage to be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_lemessage_save(LeMessage * lemessage);

/**
 * @brief Multithread-safe leauthor_save() function. 
 * 
 * @param lethread LeThread, author of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_leauthor_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe lethread_create() function. 
 * 
 * @param topic Topic (aka name) of LeThread 
 * @param lethread_id The ID of LeThread 
 * @return Pointer to the created LeThread 
 */
SharedPtr *s_lethread_create(char *topic, uint64_t lethread_id);

size_t get_min_message_size();
size_t get_max_message_size();

size_t get_min_topic_size();
size_t get_max_topic_size();

size_t get_lethread_count();

const char *get_version();

/* =============================================================================== */

/**
 * @brief Parses a query, if valid, retrieves LeThread information, including
 * topic, message history, etc. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with 
 * data==LeThread object, status==LESATATUS_OK on success. 
 * Otherwise corresponding status code is returned the structure 
 */
LeCommandResult cmd_lethread_get(char *raw_data, size_t size);

/**
 * @brief Creates LeThread with given parameters. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success. 
 * Otherwise corresponding status code is returned the structure 
 */
LeCommandResult cmd_lethread_create(char *raw_data, size_t size);

/**
 * @brief Parses a query, if valid, finds LeThread by provided parameters. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with 
 * data==Query of the filtered LeThreads, status==LESATATUS_OK on success. 
 * Otherwise corresponding status code is returned the structure 
 */
LeCommandResult cmd_lethread_find(char *raw_data, size_t size);

/**
 * @brief Parses a query, if valid, adds a new message to the specific lethread. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success. 
 * Otherwise corresponding status code is returned the structure 
 */
LeCommandResult cmd_lemessage_create(char *raw_data, size_t size);

/**
 * @brief Assures the connection is not lost. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult structure with status==LESATATUS_OK on success 
 */
LeCommandResult cmd_alive(char *raw_data, size_t size);

/**
 * @brief Returns a meta information about the server. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return LeCommandResult with meta information 
 */
LeCommandResult cmd_meta(char *raw_data, size_t size);

/**
 * @brief Tries to find specified command in the data. If command is found, executes its processor. 
 * 
 * @param raw_data Unparsed data from a client 
 * @param size raw_data size 
 * @return Result of the specific query processor. 
 * If size is not valid, returns LeCommandReuslt with status==LESTATUS_ISYN 
 */
LeCommandResult query_process(char *raw_data, size_t size);

#endif