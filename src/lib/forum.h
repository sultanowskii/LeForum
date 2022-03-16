#pragma once

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/security.h"
#include "lib/constants.h"

struct LeMessage {
	u_int64_t id;
	u_int64_t author_id;
	char *text;
	struct LeMessage *next;
};

struct LeAuthor {
	u_int64_t id;
	char token[TOKEN_LENGTH + 1];
	struct LeAuthor *next;
};

struct LeThread {
	u_int64_t id;
	u_int64_t author_id;
	u_int64_t first_message_id;
	u_int64_t last_message_id;
	u_int64_t first_participant_id;
	u_int64_t last_participant_id;
	char *topic;
	struct LeMessage *first_message;
	struct LeMessage *last_message;
	struct LeAuthor *first_participant;
	struct LeAuthor *last_participant;
};

FILE * get_thread_file(u_int64_t id, char *mode);

struct LeThread * lethread_create(char *topic, u_int64_t id);
int32_t lethread_delete(struct LeThread *thread);
struct LeMessage * lemessage_create(struct LeThread *lethread, u_int64_t author_id, char *text);
int32_t lemessage_delete(struct LeMessage *message);
struct LeAuthor * leauthor_create(struct LeThread *lethread);
int32_t leauthor_delete(struct LeAuthor *author);