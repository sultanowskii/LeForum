#ifndef LEFORUM_CLIENT_QUERY_H_
#define LEFORUM_CLIENT_QUERY_H_

#include <stdint.h>

#include "lib/constants.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/status.h"
#include "lib/util.h"

/**
 * @brief Information about query. 
 * 
 */
struct ServerQuery {
	bool_t     completed;
	void      *raw_request_data;
	size_t     raw_request_data_size;
	void   * (*parse_response)(char *raw_data, size_t size, void **result);
	void      *parsed_data;
};
typedef struct ServerQuery ServerQuery;

struct CreatedThreadInfo {
	uint64_t  thread_id;
	char     *token;
};
typedef struct CreatedThreadInfo CreatedThreadInfo;

/**
 * @brief Creates ServerQuery object with given parameters. 
 * 
 * @param parser Response parse function 
 * @param request_data Request data to be sent 
 * @param size Request data size 
 * @return Pointer to ServerQuery object 
 */ 
ServerQuery *query_create(void * (*parser)(char *, size_t, void **), char *request_data, size_t size);

/**
 * @brief Deletes ServerQuery object. 
 * Please note it doesn't delete parsed_data as long as it is needed outside.  
 * 
 * @param query ServerQuery to delete 
 * @return LESTATUS_OK on success 
 */ 
status_t query_delete(ServerQuery *query);

/**
 * @brief Generates CTHR query. 
 * 
 * @param topic Thread topic 
 * @param size Thread topic size
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_CTHR(const char *topic, size_t size);

/**
 * @brief Generates GTHR query. 
 * 
 * @param thread_id Thread ID 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_GTHR(uint64_t thread_id);

/**
 * @brief Generates FTHR query. 
 * 
 * @param topic_part Thread topic part to search for 
 * @param size Thread topic part size 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_FTHR(const char *topic_part, size_t size);

/**
 * @brief Generates CMSG query. 
 *  
 * @param thread_id Thread ID 
 * @param msg Message text 
 * @param size Message size
 * @param token Thread author token (NULL if don't have)
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_CMSG(uint64_t thread_id, const char *msg, size_t size, char *token);

/**
 * @brief Generates META query. 
 * 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_META();

/**
 * @brief Generates LIVE query. 
 * 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_LIVE();

/**
 * @brief Parses CTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @param size Size of the data
 * @param info Pointer to created thread info will be placed here on success 
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_CTHR(char *raw_data, size_t size, CreatedThreadInfo **info);

/**
 * @brief Parses GTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @param size Size of the data
 * @param lethread Pointer to retrieved lethread will be placed here on success 
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_GTHR(char *raw_data, size_t size, LeThread **lethread);

/**
 * @brief Parses FTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @param size Size of the data
 * @param found Pointer to queue with all matches will be placed here on success  
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_FTHR(char *raw_data, size_t size, Queue **found);

/**
 * @brief Parses CMSG response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @param size Size of the data
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_CMSG(char *raw_data, size_t size);

/**
 * @brief Parses META response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @param size Size of the data
 * @param meta Pointer to meta will be placed here on success
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_META(char *raw_data, size_t size, LeMeta **meta);

/**
 * @brief Parses LIVE response. 
 * 
 * @param raw_data Pointer to the raw reponse 
 * @param size Size of the data 
 * @param result Is ignored 
 * @return LESTATUS_OK on success. LESTATUS_IDAT on parsing error 
 */
status_t parse_response_LIVE(char *raw_data, size_t size, void **result);

#endif