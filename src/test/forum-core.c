#include "lib/forum.h"

int main() {
	struct LeThread *thread = lethread_create("Test Topic", rand_u_int64_t());
	struct LeMessage *message;
	u_int64_t author_id = 0;
	char *text = malloc(1024);
	size_t length = 1023;

	for (size_t i = 0; i < 3; i++) {
		printf("creator=%llu first_message_id=%llu last_message_id=%llu\n", thread->author_id, thread->first_message_id, thread->last_message_id); 
		printf("author_id text:\n > ");
		
		scanf("%llu ", &author_id);
		getline(&text, &length, stdin);
		text[strlen(text) - 1] = 0;
		
		lemessage_create(thread, author_id, text);
		message = thread->first_message;
		
		printf("==== History ====\n");
		while (message != NULL) {
			printf("#%llu: %s\n", message->author_id, message->text);
			message = message->next;
		}
		printf("=================\n");
	}

	lethread_delete(thread);
	return 0;
}