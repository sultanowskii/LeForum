#include "lib/queue.h"

struct Queue * queue_create() {
	struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));
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

int32_t queue_push(struct Queue *queue, char *data) {
	struct QueueNode *new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));

	size_t length = strlen(data);
	new_node->data = malloc(length);
	new_node->data[length - 1] = '\0';
	strncpy(new_node->data, data, length);

	if (queue->first == NULL) {
		queue->first = new_node;
		queue->last = new_node;
	}
	else {
		queue->last->next = new_node;
		queue->last = new_node;
	}

	return 0;
}

char * queue_pop(struct Queue *queue) {
	struct QueueNode *first = queue->first;
	char* data;

	// checking if the queue is empty
	if (queue_is_empty(queue) == TRUE) {
		return NULL;
	}

	// retrieving data to return
	data = first->data;
	// pop()ing the first element
	queue->first = first->next;

	// we are freeing only node itself, because the data (which is a char *) has to be returned.
	free(first);

	// if queue is empty, the last element has to point to NULL
	if (queue_is_empty(queue) == TRUE) {
		queue->last = NULL;
	}

	return data;
}

int8_t queue_is_empty(struct Queue *queue) {
	return queue->first == NULL;
}