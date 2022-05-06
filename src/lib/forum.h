#ifndef LEFORUM_LIB_FORUM_H
#define LEFORUM_LIB_FORUM_H

#include <stdint.h>
#include <stdio.h>

#include "lib/constants.h"
#include "lib/status.h"

#define TOKEN_SIZE      24

#define DIR_SERVER      ".data"
#define FILE_LETHREAD   "lethreadinfo"
#define FILE_LEMESSAGES "lemessages"
#define FILE_LEAUTHOR   "leauthor"

struct LeMessage {
	uint64_t         id;
	char            *text;
	bool_t           by_lethread_author;
	struct LeThread *lethread;
};
typedef struct LeMessage LeMessage;

struct LeAuthor {
	uint64_t  id;
	char     *token;
};
typedef struct LeAuthor LeAuthor;

struct LeThread {
	uint64_t         id;
	uint64_t         first_message_id;
	uint64_t         next_message_id;
	char            *topic;
	struct Queue    *messages;
	struct LeAuthor *author;
};
typedef struct LeThread LeThread;

/**
 * @brief Meta information about server, 
 * 
 */
struct LeMeta {
	size_t  min_message_size;
	size_t  max_message_size;
	size_t  min_topic_size;
	size_t  max_topic_size;
	char   *version;
	size_t  version_size;
	size_t  thread_count;
};
typedef struct LeMeta LeMeta;

/**
 * @brief Creates new LeThread 
 * if there is no information files with the same id. 
 * If there is, then returns LESTATUS_EXST. 
 * 
 * @param topic Topic (aka name) of LeThread 
 * @param lethread_id ID of LeThread 
 * @return Pointer to the created LeThread 
 */
LeThread *lethread_create(char *topic, uint64_t lethread_id);

/**
 * @brief Safely deletes the LeThread. 
 * Use this only if you created the LeThread using lethread_create(). 
 * 
 * @param lethread Pointer to LeThread to delete 
 * @return LESTATUS_OK on success 
 */
status_t lethread_delete(LeThread *lethread);

/**
 * @brief Creates a new LeMessage and adds it to the given LeThread. 
 * 
 * @param lethread Pointer to LeThread where new LeMessage will be posted 
 * @param text Text of the LeMessage 
 * @param by_lethread_author Is it posted by LeThread author? 
 * @return Pointer to the created LeMessage 
 */
LeMessage *lemessage_create(LeThread *lethread, char *text, bool_t by_lethread_author);

/**
 * @brief Safely deletes LeMessage. 
 * Use this only if you created the LeMessage using lemessage_create(). 
 * 
 * @param message Pointer to LeMessage to delete 
 * @return LESTATUS_OK on success 
 */
status_t lemessage_delete(LeMessage *message);

/**
 * @brief Creates a LeAuthor and adds it to the given LeThread. 
 * 
 * @param lethread Pointer to LeThread where new LeAuthor will be initialised
 * @param create_token Should token be generated for the LeAuthor? 
 * @return Pointer to the created LeAuthor 
 */
LeAuthor *leauthor_create(LeThread *lethread, bool_t create_token);

/**
 * @brief Safely deletes LeAuthor. 
 * Use this only if you created the LeMessage using leauthor_create(). 
 * 
 * @param message Pointer to LeAuthor to delete 
 * @return LESTATUS_OK on success 
 */
status_t leauthor_delete(LeAuthor *author);

/**
 * @brief Counts number of LeMessages in the given LeThread. 
 * 
 * @param lethread Pointer to LeThread to get the number of lemessages
 * @return Number of lemessages in the given lethread 
 */
uint64_t lethread_message_count(LeThread *lethread);

/**
 * @brief Opens one (specified in filename) of the lethread files. 
 * 
 * @param lethread_id ID of the lethread to open file of which 
 * @param mode File open mode 
 * @param filename Name of the file (please refer to the #defines at the top of this file) 
 * @param create Should the file be created, if it doesn't exist? 
 * @return File descriptor on success. If file is not found and create==FALSE, LESTATUS_NSFD is returned 
 */
FILE *get_lefile(uint64_t lethread_id, char *mode, char *filename, bool_t create);

/**
 * @brief Saves LeThread to the corresponding file. 
 * 
 * @param lethread Pointer to LeThread to save information about 
 * @return LESTATUS_OK on success 
 */
status_t lethread_save(LeThread *lethread);

/**
 * @brief Loads LeThread from the corresponding file. 
 * 
 * @param lethread Pointer to LeThread object where information will be loaded to 
 * @param lethread_id ID of the LeThread that is expected to be loaded 
 * @return LESTATUS_OK on success. If the corresponding file is not found, LESTATUS_NSFD is returned 
 */
status_t lethread_load(LeThread *lethread, uint64_t id);

/**
 * @brief Saves LeMessage history to the corresponding file. 
 * 
 * @param lethread Pointer to LeThread message history of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t lemessages_save(LeThread *lethread);

/**
 * @brief Saves one LeMessage to the corresponding file 
 * (appending to the end of message history file). 
 * 
 * @param lemessage Pointer to LeMessage to be saved 
 * @return LESTATUS_OK on success  
 */
status_t lemessage_save(LeMessage *lemessage);

/**
 * @brief Loads LeMessage history from the corresponding file. 
 * 
 * @param lethread Pointer to LeThread to load message history of which 
 * @return LESTATUS_OK on success. If the corresponding file is not found, LESTATUS_NSFD is returned 
 */
status_t lemessages_load(LeThread *lethread);

/**
 * @brief Loads the author of the lethread from the corresponding file.
 * 
 * @param lethread Pointer to LeThread, information about author of which will be loaded (lethread->author has to be a valid pointer to LeAuthor object)
 * @return LESTATUS_OK on success  
 */
status_t leauthor_load(LeThread *lethread);

/**
 * @brief Saves author of the lethread to the corresponding file. 
 * 
 * @param lethread Pointer to LeThread, author of which will be saved 
 * @return LESTATUS_OK on success 
 */
status_t leauthor_save(LeThread *lethread);

/**
 * @brief Comapres author token for the given LeThread and the one to check. 
 * 
 * @param lethread Pointer to LeThread, origin token of which will be checked 
 * @param token Token to check 
 * @return TRUE if the token is valid, FALSE if not 
 */
bool_t is_token_valid(LeThread *lethread, const char *token);

#endif
