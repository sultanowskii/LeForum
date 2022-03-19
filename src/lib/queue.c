#include "lib/queue.h"

/*
 * Creates a new Queue object.
 */
struct Queue * queue_create() {
	struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));

	new_queue->size = 0;
	new_queue->first = 0;
	new_queue->last = 0;

	return new_queue;
};

/*
 * Safely deletes the Queue and all the elements.
 */
int8_t queue_delete(struct Queue *queue, void (*delete_func)(void *)) {
	struct QueueNode *node = queue->first;
	struct QueueNode *next;

	while (node != NULL) {
		next = node->next;
		delete_func(node->data);
		free(node);
		node = next;
	}
	
	free(queue);

	return 0;
};

/*
 * Adds a new element to the end of the Queue.
 */
int32_t queue_push(struct Queue *queue, void *data, size_t length) {
	struct QueueNode *new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));

	new_node->data = malloc(length);
	memset(new_node->data, 0, length);
	memcpy(new_node->data, data, length);

	new_node->next = NULL;

	if (queue->first == NULL) {
		queue->first = new_node;
		queue->last = new_node;
	}
	else {
		queue->last->next = new_node;
		queue->last = new_node;
	}

	queue->size++;

	return 0;
}

/*
 * Retrieves the first element and deletes it from the Queue.
 */
void * queue_pop(struct Queue *queue) {
	struct QueueNode *first = queue->first;
	void* data;

	if (queue_is_empty(queue) == TRUE) {
		return NULL;
	}

	/* Retrieve data to return */
	data = first->data;
	/* Removing the first element */
	queue->first = first->next;

	/* free() only node itself, because the data (which is a void *) has to be returned */
	free(first);

	/* If Queue is empty, the last element has to point to NULL */
	if (queue_is_empty(queue) == TRUE) {
		queue->last = NULL;
	}

	queue->size--;

	return data;
}

/* 
 * Checks whether queue is empty or not
 */
int8_t queue_is_empty(struct Queue *queue) {
	return queue->size == 0;
}