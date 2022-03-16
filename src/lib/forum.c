#include "lib/forum.h"

struct LeThread * lethread_create(char* topic, u_int64_t id) {
	struct LeThread *new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));

	new_lethread->id = id;
	new_lethread->author_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->first_message_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->last_message_id = new_lethread->first_message_id;
	new_lethread->first_author_id = new_lethread->author_id;
	new_lethread->last_author_id = new_lethread->first_author_id;
	new_lethread->first_message = NULL;
	new_lethread->last_message = NULL;
	
	new_lethread->topic = malloc(strlen(topic));
	strncpy(new_lethread->topic, topic, strlen(topic));

	return new_lethread;
}

int32_t lethread_delete(struct LeThread *thread) {
	struct LeMessage* message = thread->first_message;
	struct LeMessage* next;

	while (message != NULL) {
		next = message->next;
		lemessage_delete(message);
		message = next;
	}

	free(thread->topic);
	free(thread);
	return 0;
}

struct LeMessage * lemessage_create(struct LeThread* lethread, u_int64_t author_id, char* text) {
	struct LeMessage* new_lemessage = (struct LeMessage *)malloc(sizeof(struct LeMessage));

	new_lemessage->author_id = author_id;
	new_lemessage->id = ++lethread->last_message_id;
	new_lemessage->next = NULL;

	size_t length = strlen(text) + 1;

	new_lemessage->text = malloc(length);
	new_lemessage->text[length - 1] = 0;
	strncpy(new_lemessage->text, text, strlen(text));

	if (lethread->first_message == NULL) {
		lethread->first_message = new_lemessage;
		lethread->last_message = new_lemessage;
	}
	else {
		lethread->last_message->next = new_lemessage;
		lethread->last_message = new_lemessage;
	}

	return new_lemessage;
}

int32_t lemessage_delete(struct LeMessage * message) {
	free(message->text);
	free(message);
	return 0;
}

struct LeAuthor * leauthor_create(struct LeThread* lethread) {
	struct LeAuthor* new_leauthor = (struct LeAuthor *)malloc(sizeof(struct LeAuthor));

	new_leauthor->id = ++lethread->last_author_id;
	rand_string(new_leauthor->token, sizeof(new_leauthor->token) - 1);
	
	return new_leauthor;
}

int32_t leauthor_delete(struct LeAuthor * author) {
	free(author);
	return 0;
}
