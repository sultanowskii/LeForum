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
	uint64_t         id;
	char             *text;
	int8_t           by_lethread_author;
};

struct LeAuthor {
	uint64_t         id;
	char*            token;
};

struct LeThread {
	uint64_t         id;
	uint64_t         first_message_id;
	uint64_t         next_message_id;
	char *           topic;
	struct Queue *   messages;
	struct LeAuthor *author;
};

struct LeThread *    lethread_create(char *topic, uint64_t lethread_id);
int32_t              lethread_delete(struct LeThread *lethread);
struct LeMessage *   lemessage_create(struct LeThread *lethread, char *text, int8_t by_lethread_author);
int32_t              lemessage_delete(struct LeMessage *message);
struct LeAuthor *    leauthor_create(struct LeThread *lethread, int8_t create_token);
int32_t              leauthor_delete(struct LeAuthor *author);

uint64_t             lethread_message_count(struct LeThread *lethread);
uint64_t             lethread_participant_count(struct LeThread *lethread);

FILE *               get_le_file(uint64_t lethread_id, char *mode, char *filename, int8_t create);

int8_t               lethread_save(struct LeThread *lethread);
int8_t               lethread_load(struct LeThread *lethread, uint64_t id);

int8_t               lemessages_save(struct LeThread *lethread);
int8_t               lemessage_save(struct LeThread *lethread, struct LeMessage *lemessage);
int8_t               lemessages_load(struct LeThread *lethread);

int8_t               leauthor_load(struct LeThread *lethread);
int8_t               leauthor_save(struct LeThread *lethread);