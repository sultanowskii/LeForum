#include "lib/queue.h"

struct Queue * queue_create() {
	struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));

	new_queue->size = 0;
	new_queue->first = 0;
	new_queue->last = 0;

	return new_queue;
};

int8_t queue_delete(struct Queue *queue) {
	struct QueueNode *node = queue->first;
	struct QueueNode *next;

	while (node != NULL) {
		next = node->next;
		free(node->data);
		free(node);
		node = next;
	}
	
	free(queue);

	return 0;
};

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

void * queue_pop(struct Queue *queue) {
	struct QueueNode *first = queue->first;
	void* data;

	// checking if the queue is empty
	if (queue_is_empty(queue) == TRUE) {
		return NULL;
	}

	// retrieving data to return
	data = first->data;
	// pop()ing the first element
	queue->first = first->next;

	// we are freeing only node itself, because the data (which is a void *) has to be returned.
	free(first);

	// if queue is empty, the last element has to point to NULL
	if (queue_is_empty(queue) == TRUE) {
		queue->last = NULL;
	}

	queue->size--;

	return data;
}

int8_t queue_is_empty(struct Queue *queue) {
	return queue->size == 0;
}