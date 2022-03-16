#pragma once

#include <string.h>
#include <stdlib.h>

#include "lib/types.h"
#include "lib/security.h"

struct LeMessage {
	u_int64_t id;
	u_int64_t author_id;
	char *text;
	struct LeMessage *next;
};

struct LeAuthor {
	u_int64_t id;
	char token[25];
};

struct LeThread {
	u_int64_t id;
	u_int64_t author_id;
	u_int64_t first_message_id;
	u_int64_t last_message_id;
	u_int64_t first_author_id;
	u_int64_t last_author_id;
	char *topic;
	struct LeMessage *first_message;
	struct LeMessage *last_message;
};

struct LeThread * lethread_create(char* topic, u_int64_t id);
int32_t lethread_delete(struct LeThread *thread);
struct LeMessage * lemessage_create(struct LeThread* lethread, u_int64_t author_id, char* text);
int32_t lemessage_delete(struct LeMessage * message);
struct LeAuthor * leauthor_create(struct LeThread* lethread);
