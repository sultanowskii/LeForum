#ifndef LEFORUM_SERVER_FORUM_IO_H_
#define LEFORUM_SERVER_FORUM_IO_H_

#include <stdio.h>

#include "lib/forum.h"
#include "lib/status.h"

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

#endif