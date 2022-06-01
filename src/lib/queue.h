#ifndef LEFORUM_LIB_QUEUE_H_
#define LEFORUM_LIB_QUEUE_H_

#include <stdlib.h>

#include "lib/constants.h"
#include "lib/status.h"

/**
 * @brief Queue that stores first/last nodes. 
 * This queue implementation can only store objects of one type. 
 * 
 */
struct Queue {
	size_t             size;
	struct QueueNode  *first;
	struct QueueNode  *last;
};
typedef struct Queue Queue;

/**
 * @brief Queue node. Please note it stores pointer itself,
 * not its value nor copy. 
 * 
 * The object is removed by calling destruct(). 
 * 
 */
struct QueueNode {
	void             *data;
	struct QueueNode *next;
};
typedef struct QueueNode QueueNode;

/**
 * @brief Creates a new Queue object. 
 * 
 * Is called for each object stored in a queue 
 * @param queue If not NULL, pointer to new queue will be placed here on success
 * @return LESTATUS_OK on success 
 */
status_t queue_create(Queue **queue);

/**
 * @brief Safely deletes the Queue and all the elements. 
 * Use this _only_ if you created queue by using `queue_create()`. 
 * 
 * @param queue Pointer to Queue to delete 
 * @param destruct Function that destroys data located on heap.
 * If `nullptr`, does nothing 
 * @return LESTATUS_OK on success 
 */
status_t queue_delete(Queue *queue, void (*destruct)(void *));

/**
 * @brief Adds a new element to the end of the Queue. 
 * 
 * @param queue Pointer to Queue where a new element will be placed 
 * @param data Pointer to data to place 
 * @return LESTATUS_OK on success 
 */
status_t queue_push(Queue *queue, void *data);

/**
 * @brief Retrieves the first element and deletes it from the Queue. 
 * 
 * @param queue Pointer to Queue to pop element from 
 * @return Pointer to the popped data. nullptr if queue is empty 
 */
void *queue_pop(Queue *queue);

/**
 * @brief Checks whether queue is empty or not. 
 * 
 * @param queue Pointer to Queue to check 
 * @return TRUE if queue is empty, otherwise ELSE 
 */
bool_t queue_is_empty(Queue *queue);

#endif