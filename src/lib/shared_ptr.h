#pragma once

#include <stdlib.h>

#include "lib/constants.h"
#include "lib/status.h"

typedef struct {
	int64_t       *ref_count;
	void          *data;
	void         (*destruct)(void *);
} SharedPtr;


/**
 * @brief Creates SharedPtr (the first one). 
 * 
 * @param data Pointer to the data 
 * @param destruct Destructor callback 
 * @return Pointer to the SharedPtr object 
 */
SharedPtr *        sharedptr_create(void *data, void (*destruct)(void *));

/**
 * @brief Creates another instance of SharedPtr. 
 * 
 * @param sharedptr The origin SharedPtr 
 * @return Pointer to the copy of SharedPtr 
 */
SharedPtr *        sharedptr_add(SharedPtr * sharedptr);

/**
 * @brief Safely deletes the SharedPtr instance.
 * 
 * @param sharedptr SharedPtr object 
 * @return LESTATUS_OK on success 
 */
status_t           sharedptr_delete(SharedPtr * sharedptr);