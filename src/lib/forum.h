#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/security.h"
#include "lib/constants.h"
#include "lib/queue.h"
#include "lib/error.h"

struct LeMessage {
	u_int64_t id;
	u_int64_t author_id;
	char *text;
};

struct LeAuthor {
	u_int64_t id;
	char token[TOKEN_LENGTH + 1];
};

struct LeThread {
	u_int64_t id;
	u_int64_t author_id;
	u_int64_t first_message_id;
	u_int64_t last_message_id;
	u_int64_t first_participant_id;
	u_int64_t last_participant_id;
	char *topic;
	struct Queue *messages;
	struct Queue *participants;
};

struct LeThread * lethread_create(char *topic, u_int64_t id);
int32_t lethread_delete(struct LeThread *lethread);
struct LeMessage * lemessage_create(struct LeThread *lethread, u_int64_t author_id, char *text);
int32_t lemessage_delete(struct LeMessage *message);
struct LeAuthor * leauthor_create(struct LeThread *lethread);
int32_t leauthor_delete(struct LeAuthor *author);

FILE * get_thread_file(u_int64_t id, char *mode, int8_t create);
int8_t lethread_save(struct LeThread *thread);
int8_t lethread_load(struct LeThread *thread, u_int64_t id);