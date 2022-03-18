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
	uint64_t id;
	uint64_t author_id;
	char *text;
};

struct LeAuthor {
	uint64_t id;
	char token[TOKEN_LENGTH + 1];
};

struct LeThread {
	uint64_t id;
	uint64_t author_id;
	uint64_t first_message_id;
	uint64_t next_message_id;
	uint64_t first_participant_id;
	uint64_t next_participant_id;
	char *topic;
	struct Queue *messages;
	struct Queue *participants;
};

struct LeThread *    lethread_create(char *topic, uint64_t lethread_id);
int32_t              lethread_delete(struct LeThread *lethread);
struct LeMessage *   lemessage_create(struct LeThread *lethread, uint64_t author_id, char *text);
int32_t              lemessage_delete(struct LeMessage *message);
struct LeAuthor *    leauthor_create(struct LeThread *lethread);
int32_t              leauthor_delete(struct LeAuthor *author);

uint64_t             lethread_message_count(struct LeThread *lethread);
uint64_t             lethread_participant_count(struct LeThread *lethread);

FILE *               get_le_file(uint64_t lethread_id, char *mode, char *filename, int8_t create);

int8_t               lethread_save(struct LeThread *lethread);
int8_t               lethread_load(struct LeThread *lethread, uint64_t id);

int8_t               lemessages_save(struct LeThread *lethread);
int8_t               lemessage_save(struct LeThread *lethread, struct LeMessage *lemessage);
int8_t               lemessages_load(struct LeThread *lethread);