#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lib/constants.h"

struct Queue {
	size_t size;
	struct QueueNode *first;
	struct QueueNode *last;
};


struct QueueNode {
	void *data;
	struct QueueNode *next;
};

struct Queue * queue_create();
int8_t queue_delete(struct Queue *queue);
int32_t queue_push(struct Queue *queue, void *data, size_t length);
void * queue_pop(struct Queue *queue);
int8_t queue_is_empty(struct Queue *queue);