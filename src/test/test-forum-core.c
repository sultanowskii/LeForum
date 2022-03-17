#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/forum.h"
#include "lib/queue.h"

int main() {
	struct stat st = {0};

	// create dir if it doesn't exist
	if (stat(".data/", &st) == -1) {
		mkdir(".data/", 0770);
	}

	u_int64_t lethread_id = rand_u_int64_t() % 0xffffffff;
	struct LeThread *lethread = lethread_create("Test Topic", lethread_id);
	struct QueueNode *node;
	struct LeMessage *message;
	u_int64_t author_id = 0;
	char *text = malloc(1024);
	size_t length = 1023;

	for (size_t i = 0; i < 5; i++) {
		printf("creator=%llu first_message_id=%llu last_message_id=%llu\n", lethread->author_id, lethread->first_message_id, lethread->last_message_id); 
		printf("author_id text:\n > ");
		
		scanf("%llu ", &author_id);
		getline(&text, &length, stdin);
		text[strlen(text) - 1] = 0;
		
		lemessage_create(lethread, author_id, text);
		node = lethread->messages->first;
		
		printf("==== History ====\n");
		while (node != NULL) {
			message = (struct LeMessage *)node->data;
			printf("#%llu: %s\n", message->author_id, message->text);
			node = node->next;
		}
		printf("=================\n");
	}

	puts("Done, deleting the lethread...");
	lethread_delete(lethread);

	puts("Trying to load the saved lethread...");
	lethread = (struct LeThread *)malloc(sizeof(struct LeThread));
	lethread_load(lethread, lethread_id);
	printf("id=%llu author_id=%llu: %s\n", lethread->id, lethread->author_id, lethread->topic);
	lethread_delete(lethread);

	return 0;
}