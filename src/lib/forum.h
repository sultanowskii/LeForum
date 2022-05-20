#ifndef LEFORUM_LIB_FORUM_H
#define LEFORUM_LIB_FORUM_H

#include <stdint.h>
#include <stdio.h>

#include "lib/constants.h"
#include "lib/status.h"

#define TOKEN_SIZE      24

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
 * @param lethread If not NULL, pointer to new lethread will be placed here on success 
 * @return LESTATUS_OK on success. LESTATUS_EXST if metafile with given ID
 * already exists 
 */
status_t lethread_create(char *topic, uint64_t lethread_id, LeThread **lethread);

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
 * @param lemessage If not NULL, pointer to new lemessage will be placed here on success 
 * @return LESTATUS_OK on success 
 */
status_t lemessage_create(LeThread *lethread, char *text, bool_t by_lethread_author, LeMessage **lemessage);

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
 * @param leauthor If not NULL, pointer to the leauthor will be placed here on success 
 * @return LESTATUS_OK on success 
 */
status_t leauthor_create(LeThread *lethread, bool_t create_token, LeAuthor **leauthor);

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
 * @brief Comapres author token for the given LeThread and the one to check. 
 * 
 * @param lethread Pointer to LeThread, origin token of which will be checked 
 * @param token Token to check 
 * @return TRUE if the token is valid, FALSE if not 
 */
bool_t is_token_valid(LeThread *lethread, const char *token);

#endif
