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
};
typedef struct LeMeta LeMeta;

/**
 * @brief Information about query 
 * 
 */
struct ServerQuery {
    bool_t    completed;
    void     *raw_request_data;
    void *  (*parse_response)(void *raw_data);
    void     *parsed_data;
};
typedef struct ServerQuery ServerQuery;

/**
 * @brief Generates CTHR query. 
 * 
 * @param topic Thread topic 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_CTHR(const char *topic);

/**
 * @brief Generates GTHR query. 
 * 
 * @param thread_id Thread ID 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_GTHR(uint64_t thread_id);

/**
 * @brief Generates FTHR query. 
 * 
 * @param topic_part Thread part to search for 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_FTHR(const char *topic_part);

/**
 * @brief Generates CMSG query. 
 *  
 * @param thread_id Thread ID 
 * @param msg Message text 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_CMSG(uint64_t thread_id, const char *msg);

/**
 * @brief Generates META query. 
 * 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_META();

/**
 * @brief Generates LIVE query. 
 * 
 * @return Pointer to the generated raw data that is ready to be sent 
 */ 
char * gen_query_LIVE();

/**
 * @brief Parses CTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return Pointer to the LeThread object with ID, name and token fields filled 
 */
LeThread * parse_response_CTHR(void *raw_data);

/**
 * @brief Parses GTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return Pointer to the LeThread object retrieved from the server 
 */
LeThread * parse_response_GTHR(void *raw_data);

/**
 * @brief Parses FTHR response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return Pointer to the queue that contains all the match threads  
 */
Queue * parse_response_FTHR(void *raw_data);

/**
 * @brief Parses CMSG response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
status_t parse_response_CMSG(void *raw_data);

/**
 * @brief Parses META response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
LeMeta * parse_response_META(void *raw_data);

/**
 * @brief Parses LIVE response. 
 * 
 * @param raw_data Pointer to the raw reponse
 * @return LESTATUS_OK on success   
 */
status_t parse_response_LIVE(void *raw_data);
