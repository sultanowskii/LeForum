#pragma once

#include "lib/constants.h"

struct LeMessage {
	uint64_t            id;
	char               *text;
	bool_t              by_lethread_author;
	struct LeThread    *lethread;
};
typedef struct LeMessage LeMessage;

struct LeAuthor {
	uint64_t            id;
	char               *token;
};
typedef struct LeAuthor LeAuthor;

struct LeThread {
	uint64_t            id;
	uint64_t            first_message_id;
	uint64_t            next_message_id;
	char               *topic;
	struct Queue       *messages;
	struct LeAuthor    *author;
};
typedef struct LeThread LeThread;