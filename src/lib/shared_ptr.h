#ifndef LEFORUM_LIB_SHARED_PTR_H_
#define LEFORUM_LIB_SHARED_PTR_H_

#include <stdint.h>
#include <stdlib.h>

#include "lib/status.h"

struct SharedPtr {
	int64_t   *ref_count;
	void      *data;
	status_t (*destruct)(void *);
};
typedef struct SharedPtr SharedPtr;

/**
 * @brief Creates SharedPtr (the first one, aka origin). 
 * 
 * @param data Pointer to the data 
 * @param destruct Destructor callback 
 * @param sharedptr If not NULL, pointer to new sharedptr will be placed here on success 
 * @return Pointer to the SharedPtr object 
 */
status_t sharedptr_create(void *data, status_t (*destruct)(void *), SharedPtr **sharedptr);

/**
 * @brief Creates another instance of SharedPtr. 
 * 
 * @param sharedptr Pointer to the origin SharedPtr 
 * @return Pointer to the copy of SharedPtr. If sharedptr is NULL, NULL is returned 
 */
SharedPtr *sharedptr_add(SharedPtr *sharedptr);

/**
 * @brief Safely deletes the SharedPtr instance. 
 * 
 * @param sharedptr Pointer to SharedPtr 
 * @return LESTATUS_OK on success 
 */
status_t sharedptr_delete(SharedPtr *sharedptr);

#endif