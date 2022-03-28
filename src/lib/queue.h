#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lib/constants.h"
#include "lib/status.h"

struct Queue {
	size_t              size;
	struct QueueNode   *first;
	struct QueueNode   *last;
};


struct QueueNode {
	void               *data;
	struct QueueNode   *next;
};

struct Queue *          queue_create();
status_t                queue_delete(struct Queue *queue, void (*delete_func)(void *));
status_t                queue_push(struct Queue *queue, void *data, size_t length);
void *                  queue_pop(struct Queue *queue);
bool_t                  queue_is_empty(struct Queue *queue);