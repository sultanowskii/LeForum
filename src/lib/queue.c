#include "lib/queue.h"

#include <stdlib.h>
#include <string.h>

#include "lib/constants.h"
#include "lib/security.h"
#include "lib/status.h"

status_t queue_create(status_t (*destruct)(void *), Queue **queue) {
	Queue *new_queue;

	NULLPTR_PREVENT(destruct, -LESTATUS_NPTR)

	new_queue = (Queue *)malloc(sizeof(*new_queue));

	new_queue->size = 0;
	new_queue->first = nullptr;
	new_queue->last = nullptr;
	new_queue->destruct = destruct;

	if (queue != nullptr)
		*queue = new_queue;
	
	new_queue = nullptr;

	return LESTATUS_OK;
};

status_t queue_delete(Queue *queue) {
	QueueNode *node;
	QueueNode *next;

	NULLPTR_PREVENT(queue, -LESTATUS_NPTR)

	node = queue->first;

	while (node != nullptr) {
		next = node->next;

		if (node->data != nullptr) {
			queue->destruct(node->data);
			node->data = nullptr;
		}

		free(node);
		node = nullptr;

		queue->size--;

		node = next;
	}

	free(queue);
	queue = nullptr;

	return LESTATUS_OK;
};

status_t queue_push(Queue *queue, void *data) {
	QueueNode *new_node;

	NULLPTR_PREVENT(queue, -LESTATUS_NPTR)
	NULLPTR_PREVENT(data, -LESTATUS_NPTR)

	new_node = (QueueNode *)malloc(sizeof(*new_node));
	new_node->data = data;

	new_node->next = nullptr;

	if (queue->first == nullptr) {
		queue->first = new_node;
		queue->last = new_node;
	}
	else {
		queue->last->next = new_node;
		queue->last = new_node;
	}

	queue->size++;

	return LESTATUS_OK;
}

void *queue_pop(Queue *queue) {
	QueueNode *first;
	void      *data;

	NULLPTR_PREVENT(queue, -LESTATUS_NPTR)

	first = queue->first;

	if (queue_is_empty(queue) == TRUE)
		return nullptr;

	/* Retrieve data to return */
	data = first->data;
	/* Removing the first element */
	queue->first = first->next;

	/* free() only node itself, because the data (which is a void *) has to be returned */
	free(first);
	first = nullptr;

	/* If Queue is empty, the last element has to point to nullptr */
	if (queue_is_empty(queue) == TRUE)
		queue->last = nullptr;

	queue->size--;

	return data;
}

inline bool_t queue_is_empty(Queue *queue) {
	NULLPTR_PREVENT(queue, -LESTATUS_NPTR)

	return queue->size == 0;
}