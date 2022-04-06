#include "lib/queue.h"

struct Queue * queue_create(void (*destruct)(void *)) {
	struct Queue            *new_queue      = (struct Queue *)malloc(sizeof(struct Queue));


	NULLPTR_PREVENT(destruct, LESTATUS_NPTR)

	new_queue->size = 0;
	new_queue->first = nullptr;
	new_queue->last = nullptr;
	new_queue->destruct = destruct;

	return new_queue;
};

status_t queue_delete(struct Queue *queue) {
	struct QueueNode        *node;
	struct QueueNode        *next;


	NULLPTR_PREVENT(queue, LESTATUS_NPTR)

	node = queue->first;
	while (node != nullptr) {
		next = node->next;

		queue->destruct(node->data);
		node->data = nullptr;

		free(node);
		node = nullptr;

		queue->size--;

		node = next;
	}

	free(queue);
	queue = nullptr;

	return LESTATUS_OK;
};

status_t queue_push(struct Queue *queue, void *data, size_t size) {
	struct QueueNode        *new_node;


	NULLPTR_PREVENT(queue, LESTATUS_NPTR)
	NULLPTR_PREVENT(data, LESTATUS_NPTR)

	new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
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

void * queue_pop(struct Queue *queue) {
	struct QueueNode        *first;
	void                    *data;


	NULLPTR_PREVENT(queue, LESTATUS_NPTR)

	first = queue->first;

	if (queue_is_empty(queue) == TRUE) {
		return nullptr;
	}

	/* Retrieve data to return */
	data = first->data;
	/* Removing the first element */
	queue->first = first->next;

	/* free() only node itself, because the data (which is a void *) has to be returned */
	free(first);
	first = nullptr;

	/* If Queue is empty, the last element has to point to nullptr */
	if (queue_is_empty(queue) == TRUE) {
		queue->last = nullptr;
	}

	queue->size--;

	return data;
}

bool_t queue_is_empty(struct Queue *queue) {
	NULLPTR_PREVENT(queue, LESTATUS_NPTR)

	return queue->size == 0;
}