#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lib/constants.h"
#include "lib/security.h"
#include "lib/status.h"

struct Queue {
	size_t              size;
	struct QueueNode   *first;
	struct QueueNode   *last;
	void              (*destruct)(void *);
};


struct QueueNode {
	void               *data;
	struct QueueNode   *next;
};

/**
 * @brief Creates a new Queue object. 
 * 
 * @param destruct Callback that safely deletes one object stored in a queue. Is called for each object stored in a queue 
 * @return Pointer to created Queue 
 */
struct Queue *          queue_create(void (*destruct)(void *));

/**
 * @brief Safely deletes the Queue and all the elements. 
 * 
 * @param queue Queue to delete 
 * @return LESTATUS_OK on success 
 */
status_t                queue_delete(struct Queue *queue);

/**
 * @brief Adds a new element to the end of the Queue. 
 * 
 * @param queue Queue where a new element will be placed 
 * @param data Object to place 
 * @param size Size of the object 
 * @return LESTATUS_OK on success 
 */
status_t                queue_push(struct Queue *queue, void *data, size_t size);

/**
 * @brief Retrieves the first element and deletes it from the Queue. 
 * 
 * @param queue Queue to pop element from 
 * @return Pointer to the popped object 
 */
void *                  queue_pop(struct Queue *queue);

/**
 * @brief Checks whether queue is empty or not 
 * 
 * @param queue Queue to check 
 * @return TRUE if queue is empty, otherwise ELSE 
 */
bool_t                  queue_is_empty(struct Queue *queue);