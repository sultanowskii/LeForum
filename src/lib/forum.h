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
#include "lib/status.h"

#define TOKEN_SIZE           24

#define DATA_DIR             ".data"
#define FILENAME_LETHREAD    "lethreadinfo"
#define FILENAME_LEMESSAGES  "lemessages"
#define FILENAME_LEAUTHOR    "leauthor"

struct LeMessage {
	uint64_t            id;
	char               *text;
	bool_t              by_lethread_author;
	struct LeThread    *lethread;
};

struct LeAuthor {
	uint64_t            id;
	char               *token;
};

struct LeThread {
	uint64_t            id;
	uint64_t            first_message_id;
	uint64_t            next_message_id;
	char               *topic;
	struct Queue       *messages;
	struct LeAuthor    *author;
};

struct LeThread *       lethread_create(char *topic, uint64_t lethread_id);
status_t                lethread_delete(struct LeThread *lethread);
struct LeMessage *      lemessage_create(struct LeThread *lethread, char *text, bool_t by_lethread_author);
status_t                lemessage_delete(struct LeMessage *message);
struct LeAuthor *       leauthor_create(struct LeThread *lethread, bool_t create_token);
status_t                leauthor_delete(struct LeAuthor *author);

uint64_t                lethread_message_count(struct LeThread *lethread);
uint64_t                lethread_participant_count(struct LeThread *lethread);

FILE *                  get_le_file(uint64_t lethread_id, char *mode, char *filename, bool_t create);

status_t                lethread_save(struct LeThread *lethread);
status_t                lethread_load(struct LeThread *lethread, uint64_t id);

status_t                lemessages_save(struct LeThread *lethread);
status_t                lemessage_save(struct LeMessage *lemessage);
status_t                lemessages_load(struct LeThread *lethread);

status_t                leauthor_load(struct LeThread *lethread);
status_t                leauthor_save(struct LeThread *lethread);

bool_t                  is_token_valid(struct LeThread *lethread, const char *token);