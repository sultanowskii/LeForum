#include "lib/forum.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "global/conf.h"
#include "lib/constants.h"
#include "lib/queue.h"
#include "lib/security.h"
#include "lib/status.h"
#include "lib/util.h"

status_t lethread_create(char *topic, uint64_t lethread_id, LeThread **lethread) {
	LeThread *new_lethread;
	size_t    topic_size;

	NULLPTR_PREVENT(topic, -LESTATUS_NPTR)

	new_lethread = (LeThread *)malloc(sizeof(*new_lethread));
	topic_size = strlen(topic);

	new_lethread->id = lethread_id;
	new_lethread->first_message_id = rand_uint64_t() % 0xffffffff;
	new_lethread->next_message_id = new_lethread->first_message_id;
	queue_create(lemessage_delete, &new_lethread->messages);
	new_lethread->author = nullptr;

	new_lethread->topic = calloc(sizeof(char), topic_size + 1);
	strncpy(new_lethread->topic, topic, topic_size);

	if (lethread != nullptr)
		*lethread = new_lethread;

	new_lethread = 0;

	return LESTATUS_OK;
}

status_t lethread_delete(LeThread *lethread) {
	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	if (lethread->messages != nullptr) {
		queue_delete(lethread->messages);
		lethread->messages = nullptr;
	}

	if (lethread->author != nullptr) {
		leauthor_delete(lethread->author);
		lethread->author = nullptr;
	}

	if (lethread->topic != nullptr) {
		free(lethread->topic);
		lethread->topic = nullptr;
	}

	free(lethread);
	lethread = nullptr;

	return LESTATUS_OK;
}

uint64_t lethread_message_count(LeThread *lethread) {
	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	return lethread->next_message_id - lethread->first_message_id;
}

status_t lemessage_create(LeThread *lethread, char *text, bool_t by_lethread_author, LeMessage **lemessage) {
	LeMessage *new_lemessage;
	size_t     length;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->messages, -LESTATUS_NPTR)
	NULLPTR_PREVENT(text, -LESTATUS_NPTR)

	length = strlen(text);

	if (length < MIN_MESSAGE_SIZE || length > MAX_MESSAGE_SIZE)
		return -LESTATUS_IDAT;

	new_lemessage = (LeMessage *)malloc(sizeof(*new_lemessage));

	new_lemessage->id = lethread->next_message_id++;
	new_lemessage->by_lethread_author = by_lethread_author;

	new_lemessage->text = malloc(length + 1);
	strncpy(new_lemessage->text, text, length);
	new_lemessage->text[length] = '\0';
	new_lemessage->lethread = lethread;

	queue_push(lethread->messages, new_lemessage);

	if (lemessage != nullptr)
		*lemessage = new_lemessage;

	new_lemessage = nullptr;

	return LESTATUS_OK;
}

status_t lemessage_delete(LeMessage *message) {
	NULLPTR_PREVENT(message, -LESTATUS_NPTR)

	if (message->text != nullptr) {
		 free(message->text);
		 message->text = nullptr;
	}

	free(message);
	message = nullptr;

	return LESTATUS_OK;
}

status_t leauthor_create(LeThread *lethread, bool_t create_token, LeAuthor **leauthor) {
	LeAuthor *new_leauthor;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	new_leauthor = (LeAuthor *)malloc(sizeof(*new_leauthor));
	new_leauthor->id = rand_uint64_t() % 0xffffffff;

	if (create_token)
		new_leauthor->token = rand_string(TOKEN_SIZE);
	else
		new_leauthor->token = calloc(sizeof(char), TOKEN_SIZE + 1);

	lethread->author = new_leauthor;

	if (leauthor != nullptr)
		*leauthor = new_leauthor;

	new_leauthor = nullptr;

	return LESTATUS_OK;
}

status_t leauthor_delete(LeAuthor *author) {
	NULLPTR_PREVENT(author, -LESTATUS_NPTR)

	if (author->token != nullptr) {
		free(author->token);
		author->token = nullptr;
	}

	free(author);
	author = nullptr;

	return LESTATUS_OK;
}

bool_t is_token_valid(LeThread *lethread, const char *token) {
	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author->token, -LESTATUS_NPTR)
	NULLPTR_PREVENT(token, -LESTATUS_NPTR)

	if (strncmp(lethread->author->token, token, TOKEN_SIZE) != 0)
		return FALSE;
	return TRUE;
}