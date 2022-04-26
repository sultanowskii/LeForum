#pragma once

#include <stdio.h>

#include "lib/forum.h"
#include "lib/queue.h"

/**
 * @brief Meta information about server 
 * 
 */
struct LeMeta {
    size_t    min_message_size;
    size_t    max_message_size;
    size_t    min_topic_size;
    size_t    max_topic_size;
    char     *version;
    size_t    version_size;
};
typedef struct LeMeta LeMeta;

/**
 * @brief Information about query 
 * 
 */
struct ServerQuery {
    bool_t    completed;
    void     *raw_request_data;
    size_t    raw_request_data_size;
    void *  (*parse_response)(char *raw_data);
    void     *parsed_data;
};
typedef struct ServerQuery ServerQuery;

struct LeData {
    void     *data;
    size_t    size;
};
typedef struct LeData LeData;

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
ServerQuery * query_create(void * (*parser)(char *raw_data), char *request_data, size_t size);

/**
 * @brief Deletes ServerQuery object. Please note it doesn't delete parsed_data as long as it is needed outside.  
 * 
 * @param query ServerQuery to delete 
 * @return LESTATUS_OK on success 
 */ 
status_t query_delete(ServerQuery *query);

/**
 * @brief Generates CTHR query. 
 * 
 * @param topic Thread topic 
 * @param topic_size Thread topic size
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
LeData gen_query_CTHR(const char *topic, size_t topic_size);

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
 * @return Pointer to the LeThread object with ID, name and token fields filled 
 */
CreatedThreadInfo * parse_response_CTHR(char *raw_data);

/**
 * @brief Parses GTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return Pointer to the LeThread object retrieved from the server 
 */
LeThread * parse_response_GTHR(char *raw_data);

/**
 * @brief Parses FTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return Pointer to the queue that contains all the match threads  
 */
Queue * parse_response_FTHR(char *raw_data);

/**
 * @brief Parses CMSG response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
status_t parse_response_CMSG(char *raw_data);

/**
 * @brief Parses META response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
LeMeta * parse_response_META(char *raw_data);

/**
 * @brief Parses LIVE response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
status_t parse_response_LIVE(char *raw_data);
