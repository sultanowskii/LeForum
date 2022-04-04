#include "server/forum.h"

struct LeThread * lethread_create(char *topic, uint64_t lethread_id) {
	struct LeThread         *new_lethread;
	size_t                   topic_length;
	FILE*                    lethread_file  = get_le_file(lethread_id, "rb", FILENAME_LETHREAD, FALSE);


	/* If the lethread file already exists, then nothing should be done */
	if (lethread_file != LESTATUS_NSFD) {
		fclose(lethread_file);
		return LESTATUS_EXST;
	}

	new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));
	topic_length = strlen(topic);

	new_lethread->id = lethread_id;
	new_lethread->first_message_id = rand_uint64_t() % 0xffffffff;
	new_lethread->next_message_id = new_lethread->first_message_id;
	new_lethread->messages = queue_create();
	new_lethread->author = NULL;

	new_lethread->topic = malloc(topic_length + 1);
	memset(new_lethread->topic, 0, topic_length + 1);
	strncpy(new_lethread->topic, topic, topic_length);

	return new_lethread;
}

status_t lethread_delete(struct LeThread *lethread) {
	struct QueueNode        *node           = lethread->messages->first;
	struct LeMessage        *lemessage;


	queue_delete(lethread->messages, (void (*)(void *))lemessage_delete);

	leauthor_delete(lethread->author);

	free(lethread->topic);
	free(lethread);

	return LESTATUS_OK;
}

uint64_t lethread_message_count(struct LeThread *lethread) {
	return lethread->next_message_id - lethread->first_message_id;
}

struct LeMessage * lemessage_create(struct LeThread *lethread, char *text, bool_t by_lethread_author) {
	struct LeMessage        *new_lemessage  = (struct LeMessage *)malloc(sizeof(struct LeMessage));
	size_t                   length         = strlen(text);


	new_lemessage->id = lethread->next_message_id++;
	new_lemessage->by_lethread_author = by_lethread_author;

	new_lemessage->text = malloc(length + 1);
	strncpy(new_lemessage->text, text, length);
	new_lemessage->text[length] = '\0';
	new_lemessage->lethread = lethread;

	queue_push(lethread->messages, new_lemessage, sizeof(struct LeMessage));
	free(new_lemessage);

	return lethread->messages->last->data;
}

status_t lemessage_delete(struct LeMessage *message) {
	free(message->text);
	free(message);

	return LESTATUS_OK;
}

struct LeAuthor * leauthor_create(struct LeThread *lethread, bool_t create_token) {
	struct LeAuthor         *new_leauthor   = (struct LeAuthor *)malloc(sizeof(struct LeAuthor));

	new_leauthor->id = rand_uint64_t() % 0xffffffff;
	new_leauthor->token = malloc(TOKEN_SIZE + 1);
	memset(new_leauthor->token, 0, TOKEN_SIZE + 1);
	if (create_token) rand_string(new_leauthor->token, TOKEN_SIZE);

	lethread->author = new_leauthor;

	return new_leauthor;
}

status_t leauthor_delete(struct LeAuthor *author) {
	free(author->token);
	free(author);

	return LESTATUS_OK;
}

FILE * get_le_file(uint64_t lethread_id, char *mode, char *filename, bool_t create) {
	char                     path[256];
	char                     id_str[32];

	FILE                    *file;
	struct stat              st             = {0};


	memset(id_str, 0, sizeof(id_str));
	memset(path, 0, sizeof(path));

	sprintf(path, DATA_DIR "/%llu/", lethread_id);

	/* Check if the directory exists */
	if (stat(path, &st) == -1) {
		if (!create) return LESTATUS_NSFD;
		mkdir(path, 0700);
	}

	strcat(path, filename);

	/* Check if the file exists */
	if (stat(path, &st) == -1 && !create) {
		return LESTATUS_NSFD;
	}

	file = fopen(path, mode);

	return file;
}

status_t lethread_save(struct LeThread *lethread) {
	size_t                   topic_length   = strlen(lethread->topic);
	FILE                    *lethread_info_file;


	/* This trick clears the file so we don't have to have a headache with all these overwriting file stuff */
	lethread_info_file = get_le_file(lethread->id, "wb", FILENAME_LETHREAD, TRUE);
	fclose(lethread_info_file);

	lethread_info_file = get_le_file(lethread->id, "ab", FILENAME_LETHREAD, TRUE);

	fwrite(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, lethread_info_file);
	fwrite(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fwrite(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fwrite(&topic_length, sizeof(topic_length), 1, lethread_info_file);
	fwrite(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);

	return LESTATUS_OK;
}

status_t lethread_load(struct LeThread *lethread, uint64_t lethread_id) {
	FILE                    *lethread_info_file  = get_le_file(lethread_id, "rb", FILENAME_LETHREAD, FALSE);
	size_t                   topic_length;
	uint64_t                 leauthor_id;


	if (lethread_info_file == LESTATUS_NSFD) {
		return LESTATUS_NSFD;
	}

	lethread->messages = queue_create();
	lethread->author = NULL;

	fread(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fread(&leauthor_id, sizeof(lethread->author->id), 1, lethread_info_file);
	fread(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fread(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fread(&topic_length, sizeof(topic_length), 1, lethread_info_file);

	lethread->topic = malloc(topic_length + 1);
	memset(lethread->topic, 0, topic_length + 1);

	fread(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);

	return LESTATUS_OK;
}

status_t lemessages_save(struct LeThread *lethread) {
	FILE                    *lemessages_file;

	struct QueueNode        *node                = lethread->messages->first;

	struct LeMessage        *lemessage;
	size_t                   text_length;


	/* This trick clears the file so we don't have to have a headache with all these overwriting file stuff */
	lemessages_file = get_le_file(lethread->id, "wb", FILENAME_LEMESSAGES, TRUE);
	fclose(lemessages_file);

	lemessages_file = get_le_file(lethread->id, "ab", FILENAME_LEMESSAGES, TRUE);

	while (node != NULL) {
		lemessage = node->data;
		text_length = strlen(lemessage->text);
		fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
		fwrite(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
		fwrite(&text_length, sizeof(text_length), 1, lemessages_file);
		fwrite(lemessage->text, 1, text_length, lemessages_file);
		node = node->next;
	}

	fclose(lemessages_file);

	return LESTATUS_OK;
}

status_t lemessage_save(struct LeMessage *lemessage) {
	size_t                   text_length         = strlen(lemessage->text);
	FILE                    *lemessages_file;


	lemessages_file = get_le_file(lemessage->lethread->id, "ab", FILENAME_LEMESSAGES, TRUE);

	fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
	fwrite(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
	fwrite(&text_length, sizeof(text_length), 1, lemessages_file);
	fwrite(lemessage->text, 1, text_length, lemessages_file);

	fclose(lemessages_file);

	return LESTATUS_OK;
}

status_t lemessages_load(struct LeThread *lethread) {
	FILE                    *lemessages_file     = get_le_file(lethread->id, "rb", FILENAME_LEMESSAGES, FALSE);

	struct LeMessage         lemessage           = {0};
	size_t                   text_length;


	if (lemessages_file == LESTATUS_NSFD) {
		return LESTATUS_NSFD;
	}

	for (size_t i = 0; i < lethread_message_count(lethread); ++i) {
		fread(&lemessage.id, sizeof(lemessage.id), 1, lemessages_file);
		fread(&lemessage.by_lethread_author, sizeof(lemessage.by_lethread_author), 1, lemessages_file);
		fread(&text_length, sizeof(text_length), 1, lemessages_file);

		lemessage.text = malloc(text_length + 1);
		memset(lemessage.text, 0, text_length + 1);
		fread(lemessage.text, 1, text_length, lemessages_file);

		queue_push(lethread->messages, &lemessage, sizeof(struct LeMessage));
	}

	fclose(lemessages_file);

	return LESTATUS_OK;
}

status_t leauthor_load(struct LeThread *lethread) {
	struct LeAuthor         *leauthor;
	FILE                    *leauthor_file  = get_le_file(lethread->id, "rb", FILENAME_LEAUTHOR, FALSE);

	if (leauthor_file == LESTATUS_NSFD) {
		return LESTATUS_NSFD;
	}

	leauthor = leauthor_create(lethread, FALSE);

	fread(&leauthor->id, sizeof(leauthor->id), 1, leauthor_file);
	fread(leauthor->token, 1, TOKEN_SIZE, leauthor_file);

	fclose(leauthor_file);

	return LESTATUS_OK;
}

status_t leauthor_save(struct LeThread *lethread) {
	FILE                    *leauthor_file  = get_le_file(lethread->id, "wb", FILENAME_LEAUTHOR, TRUE);


	fclose(leauthor_file);

	leauthor_file = get_le_file(lethread->id, "ab", FILENAME_LEAUTHOR, TRUE);

	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, leauthor_file);
	fwrite(lethread->author->token, TOKEN_SIZE, 1, leauthor_file);

	fclose(leauthor_file);

	return LESTATUS_OK;
}

bool_t is_token_valid(struct LeThread *lethread, const char *token) {
	if (strncmp(lethread->author->token, token, TOKEN_SIZE) != 0) {
		return FALSE;
	}

	return TRUE;
}