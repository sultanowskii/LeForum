#include "lib/forum.h"

FILE * get_thread_file(u_int64_t id, char *mode) {
	char filename[256];
	char id_str[32];

	struct stat st = {0};

	memset(id_str, 0, sizeof(id_str));
	memset(filename, 0, sizeof(filename));

	strcat(filename, ".data/");
	sprintf(id_str, "%llu", id);
	strcat(filename, id_str);
	
	if (stat(filename, &st) == -1) {
		mkdir(filename, 0700);
	}

	strcat(filename, "/thread");
	FILE *thread_info_file = fopen(filename, mode);
	return thread_info_file;
}

struct LeThread * lethread_create(char *topic, u_int64_t id) {
	struct LeThread *new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));
	size_t topic_length = strlen(topic);

	new_lethread->id = id;
	new_lethread->author_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->first_message_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->last_message_id = new_lethread->first_message_id;
	new_lethread->first_participant_id = new_lethread->author_id;
	new_lethread->last_participant_id = new_lethread->first_participant_id;
	new_lethread->first_message = NULL;
	new_lethread->last_message = NULL;
	
	new_lethread->topic = malloc(topic_length);
	strncpy(new_lethread->topic, topic, topic_length);

	// save the thread information to the file
	FILE *thread_info_file = get_thread_filename(id, "ab");
	fwrite(&new_lethread->id, sizeof(new_lethread->id), 1, thread_info_file);
	fwrite(&new_lethread->author_id, sizeof(new_lethread->author_id), 1, thread_info_file);
	fwrite(&new_lethread->first_message_id, sizeof(new_lethread->first_message_id), 1, thread_info_file);
	fwrite(&new_lethread->last_message_id, sizeof(new_lethread->last_message_id), 1, thread_info_file);
	fwrite(&new_lethread->first_participant_id, sizeof(new_lethread->first_participant_id), 1, thread_info_file);
	fwrite(&new_lethread->last_participant_id, sizeof(new_lethread->last_participant_id), 1, thread_info_file);
	fwrite(&topic_length, sizeof(topic_length), 1, thread_info_file);
	fwrite(topic, 1, topic_length, thread_info_file);
	fclose(thread_info_file);

	return new_lethread;
}

int32_t lethread_delete(struct LeThread *thread) {
	struct LeMessage *message = thread->first_message;
	struct LeMessage *next;

	while (message != NULL) {
		next = message->next;
		lemessage_delete(message);
		message = next;
	}

	free(thread->topic);
	free(thread);
	return 0;
}

struct LeMessage * lemessage_create(struct LeThread *lethread, u_int64_t author_id, char *text) {
	struct LeMessage *new_lemessage = (struct LeMessage *)malloc(sizeof(struct LeMessage));

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

int32_t lemessage_delete(struct LeMessage *message) {
	free(message->text);
	free(message);
	return 0;
}

struct LeAuthor * leauthor_create(struct LeThread *lethread) {
	struct LeAuthor *new_leauthor = (struct LeAuthor *)malloc(sizeof(struct LeAuthor));

	new_leauthor->id = ++lethread->last_participant_id;
	rand_string(new_leauthor->token, sizeof(new_leauthor->token) - 1);
	
	return new_leauthor;
}

int32_t leauthor_delete(struct LeAuthor *author) {
	free(author);
	return 0;
}
