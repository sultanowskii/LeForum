#include "lib/forum.h"

struct LeThread * lethread_create(char* topic, u_int64_t id) {
	struct LeThread *new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));

	new_lethread->id = id;
	new_lethread->author_id = rand() % 0xffffffff;
	new_lethread->first_message_id = rand() % 0xffffffff;
	new_lethread->last_message_id = new_lethread->first_message_id;
	new_lethread->first_author_id = new_lethread->author_id;
	new_lethread->last_author_id = new_lethread->first_author_id;
	new_lethread->first_message = NULL;
	new_lethread->last_message = NULL;
	
	new_lethread->topic = malloc(strlen(topic));
	strncpy(new_lethread->topic, topic, strlen(topic));

	return new_lethread;
}

struct LeMessage * lemessage_create(struct LeThread* lethread, u_int64_t author_id, char* text) {
	struct LeMessage* new_lemessage = (struct LeMessage *)malloc(sizeof(struct LeMessage));

	new_lemessage->author_id = author_id;
	new_lemessage->id = ++lethread->last_message_id;
	new_lemessage->next = NULL;

	new_lemessage->text = malloc(strlen(text));
	strncpy(new_lemessage->text, text, strlen(text));

	if (lethread->last_message_id == lethread->first_message_id) {
		lethread->first_message = new_lemessage;
	}
	lethread->last_message = new_lemessage;
	return new_lemessage;
}

struct LeAuthor * leauthor_create(struct LeThread* lethread) {
	struct LeAuthor* new_leauthor = (struct LeMessage *)malloc(sizeof(struct LeAuthor));

	new_leauthor->id = ++lethread->last_author_id;
	return new_leauthor;
}