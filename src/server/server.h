#ifndef LEFORUM_SERVER_SERVER_H_
#define LEFORUM_SERVER_SERVER_H_

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "global/conf.h"
#include "lib/forum.h"
#include "lib/status.h"
#include "lib/shared_ptr.h"
#include "server/forum_io.h"

#define FILE_LEMETA "lemeta"

/**
 * handle_client() argument, 
 */
struct LeClientInfo {
	int32_t            fd;
	struct sockaddr_in addr;
	socklen_t          addr_size;
};
typedef struct LeClientInfo LeClientInfo;

/**
 * @brief Main program logic, initialises all the threads, sets callbacks, 
 * binds a socket, accepts client connections. 
 * 
 * @param argc Number of arguments 
 * @param argv Arguments 
 * @return Status of program termination 
 */
status_t main(int argc, char *argv[]);

/**
 * @brief Communicates with a client, gets and sends queries and requests. 
 * 
 * @param arg LeClientInfo pointer. 
 * Has to contain information about client (please refer to 
 * the LeClientInfo structure defenition at the top of this file) 
 * @return Nothing 
 */
void *handle_client(void *arg);

/**
 * @brief Loads already existing LeThreads to the queue. 
 * Notice that this fucntion doesn't load lemessages - we load them when we are
 * asked to by client for the first time.
 * 
 * @return Number of LeThreads loaded 
 */
size_t startup();

/**
 * @brief free()s allocated data, saves essential information. 
 */
void cleanup();

/**
 * @brief Loads program metadata. 
 */
void lemeta_load();

/**
 * @brief Saves program metadata. 
 */
void lemeta_save();

/**
 * @brief Returns number of loaded lethreads. 
 * 
 * @returns Number of loaded lethreads
 */
size_t get_lethread_count();

/**
 * @brief Saves LeThreads (to corresponding files). 
 * LeThreads to save are provided via speciall queue. 
 * 
 * This function has to run in the separate thread. 
 * The purpose is to avoid accessing same file from different threads. 
 * 
 * @return Nothing 
 */
void *lethread_query_manage();

/**
 * @brief Saves LeMessage histories (to corresponding files). 
 * LeThreads, which history is to save are provided via speciall queue. 
 *
 * This function has to run in the separate thread. 
 * The purpose is to avoid accessing same file from different threads. 
 * 
 * @return Nothing 
 */
void *lemessages_query_manage();

/**
 * @brief Saves LeMessages (to corresponding files). 
 * LeMessages to save are provided via speciall queue. 
 * 
 * This function has to run in the separate thread. 
 * The purpose is to avoid accessing same file from different threads. 
 * 
 * @return Nothing 
 */
void *lemessage_query_manage();

/**
 * @brief Saves LeAuthors (to corresponding files). 
 * LeAuthors to save are provided via speciall queue. 
 * 
 * This function has to run in the separate thread. 
 * The purpose is to avoid accessing same file from different threads. 
 * 
 * @return Nothing 
 */
void *leauthor_query_manage();

/**
 * @brief Multithread-safe lethread_save() function. 
 * The returned LeThread has to be free()'d by you as a user of this interface. 
 * 
 * @param lethread LeThread to save information about 
 * @return LESTATUS_OK on success  
 */
status_t s_lethread_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe s_lemessages_save() function. 
 * 
 * @param lethread Pointer to LeThread message history of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_lemessages_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Multithread-safe s_lemessage_save() function. 
 * 
 * @param lemessage LeMessage to be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_lemessage_save(struct LeMessage * lemessage);

/**
 * @brief Implementation of s_leauthour_save() required by query.h. 
 * 
 * @param lethread LeThread, author of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t s_leauthor_save(SharedPtr *sharedptr_lethread);

/**
 * @brief Implementation of s_lethread_create() required by query.h. 
 * 
 * @param topic Topic (aka name) of LeThread 
 * @param lethread_id The ID of LeThread 
 * @return Pointer to the created LeThread 
 */
SharedPtr *s_lethread_create(char *topic, uint64_t lethread_id);

/**
 * @brief Implementation of lethread_get_by_id() required by query.h. 
 *
 * @param lethread_id ID of LeThread to search for 
 * @return LeThread with the given ID. If not found, LESTATUS_NFND is returned 
 */
SharedPtr *lethread_get_by_id(uint64_t lethread_id);

/**
 * @brief Callback for queue_delete(). 
 * 
 * @param clinfo Pointer to LeClientInfo structure
 */
status_t leclientinfo_delete(struct LeClientInfo *clinfo);

/**
 * @brief Callback for signal(). Cleans up and exits if some signal occurs.  
 * 
 * @param signum Signal number 
 */
void signal_handler(const int signum);

/**
 * @brief Safely increments next_lethread_id_value. 
 * 
 * @return The value of next_lethread_id_value before the increment 
 */
uint64_t next_lethread_id();

#endif