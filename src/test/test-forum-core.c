#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "server/core/forum.h"

#include "lib/queue.h"
#include "lib/security.h"
#include "lib/shared_ptr.h"

void lethread_info(struct LeThread *lethread) {
	printf("LeThread: id=%llu author_id=%llu first_message_id=%llu next_message_id=%llu: %s\n", lethread->id, lethread->author->id, lethread->first_message_id, lethread->next_message_id, lethread->topic);
}

void lethread_message_history(struct LeThread *lethread) {
	struct QueueNode        *node;
	struct LeMessage        *message;


	node = lethread->messages->first;

	printf("==== History ====\n");
	while (node != NULL) {
		message = (struct LeMessage *)node->data;
		printf("#%llu: %s\n", message->id, message->text);
		node = node->next;
	}
	printf("=================\n");
}

status_t main() {
	struct stat              st             = {0};

	uint64_t                 lethread_id    = rand_uint64_t() % 0xffffffff;
	struct LeThread         *lethread       = lethread_create("Test Topic", lethread_id);

	struct LeMessage        *message;
	char                    *text           = malloc(1024);
	size_t                   text_size      = 1023;

	struct LeAuthor         *leauthor       = leauthor_create(lethread, TRUE);
	uint64_t                 author_id      = 0;


	leauthor_save(lethread);

	lethread_save(lethread);

	/* create dir if it doesn't exist */
	if (stat(".data/", &st) == -1) {
		mkdir(".data/", 0770);
	}

	for (size_t i = 0; i < 5; i++) {
		lethread_info(lethread);
		printf("author_id text:\n > ");

		scanf("%llu ", &author_id);
		getline(&text, &text_size, stdin);
		text[strlen(text) - 1] = 0;

		message = lemessage_create(lethread, text, rand_uint64_t() % 2);

		lemessage_save(message);
		lethread_save(lethread);

		lethread_message_history(lethread);
	}

	puts("Done, saving to file, deleting object...");

	lethread_save(lethread);
	lemessages_save(lethread);
	leauthor_save(lethread);
	lethread_delete(lethread);
	lethread = nullptr;

	puts("Trying to load the saved lethread...");

	lethread = (struct LeThread *)malloc(sizeof(struct LeThread));

	lethread_load(lethread, lethread_id);
	lemessages_load(lethread);
	leauthor_load(lethread);

	printf("LeAuthor: id=%llu, token=%s !!!\n", lethread->author->id, lethread->author->token);

	lethread_info(lethread);
	lethread_message_history(lethread);

	lethread_delete(lethread);
	lethread = nullptr;
	free(text);
	text = nullptr;

	puts("Done!");

	return LESTATUS_OK;
}