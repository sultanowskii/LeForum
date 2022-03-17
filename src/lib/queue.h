#pragma once

#include <stdlib.h>
#include <string.h>

#include "lib/constants.h"

struct Queue {
	struct QueueNode *first;
	struct QueueNode *last;
};


struct QueueNode {
	char *data;
	struct QueueNode *next;
};

struct Queue * queue_create();
int8_t queue_delete(struct Queue *queue);
int32_t queue_push(struct Queue *queue, char *data);
char * queue_pop(struct Queue *queue);
int8_t queue_is_empty(struct Queue *queue);