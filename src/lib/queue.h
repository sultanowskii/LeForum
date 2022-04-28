#ifndef LEFORUM_LIB_QUEUE_H_
#define LEFORUM_LIB_QUEUE_H_

#include <stdlib.h>

#include "lib/constants.h"
#include "lib/status.h"

struct Queue {
	size_t              size;
	struct QueueNode   *first;
	struct QueueNode   *last;
	status_t          (*destruct)(void *);
};
typedef struct Queue Queue;


struct QueueNode {
	void               *data;
	struct QueueNode   *next;
};
typedef struct QueueNode QueueNode;

/**
 * @brief Creates a new Queue object. 
 * 
 * @param destruct Callback that safely deletes one object stored in a queue. Is called for each object stored in a queue 
 * @return Pointer to created Queue 
 */
Queue *            queue_create(status_t (*destruct)(void *));

/**
 * @brief Safely deletes the Queue and all the elements. 
 * Use this _only_ if you created queue by using `queue_create()`.
 * 
 * @param queue Pointer to Queue to delete 
 * @return LESTATUS_OK on success 
 */
status_t           queue_delete(Queue *queue);

/**
 * @brief Adds a new element to the end of the Queue. 
 * 
 * @param queue Pointer to Queue where a new element will be placed 
 * @param data Pointer to data to place 
 * @param size Size of the data 
 * @return LESTATUS_OK on success 
 */
status_t           queue_push(Queue *queue, void *data, size_t size);

/**
 * @brief Retrieves the first element and deletes it from the Queue. 
 * 
 * @param queue Pointer to Queue to pop element from 
 * @return Pointer to the popped data. nullptr if queue is empty 
 */
void *             queue_pop(Queue *queue);

/**
 * @brief Checks whether queue is empty or not. 
 * 
 * @param queue Pointer to Queue to check 
 * @return TRUE if queue is empty, otherwise ELSE 
 */
bool_t             queue_is_empty(Queue *queue);

#endif