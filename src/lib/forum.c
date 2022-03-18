#include "lib/forum.h"

/*
 * Creates new LeThread, if there is no one with provided id.
 * If there is, then returns ERREXST.
 */
struct LeThread * lethread_create(char *topic, uint64_t lethread) {
	// If the lethread file already exists, then nothing should be done
	FILE* lethread_file = get_le_file(lethread, "rb", FILENAME_LETHREAD, FALSE);
	if (lethread_file != ERRNSFD) {
		fclose(lethread_file);
		return ERREXST;
	}
	
	struct LeThread *new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));
	size_t topic_length = strlen(topic);

	new_lethread->id = lethread;
	new_lethread->author_id = rand_uint64_t() % 0xffffffff;
	new_lethread->first_message_id = rand_uint64_t() % 0xffffffff;
	new_lethread->next_message_id = new_lethread->first_message_id;
	new_lethread->first_participant_id = new_lethread->author_id;
	new_lethread->next_participant_id = new_lethread->first_participant_id;
	new_lethread->messages = queue_create();
	new_lethread->participants = queue_create();
	
	new_lethread->topic = malloc(topic_length + 1);
	memset(new_lethread->topic, 0, topic_length + 1);
	strncpy(new_lethread->topic, topic, topic_length);

	lethread_save(new_lethread);

	return new_lethread;
}


/*
 * Safely deletes the LeThread.
 */
int32_t lethread_delete(struct LeThread *lethread) {
	queue_delete(lethread->messages);
	queue_delete(lethread->participants);
	free(lethread->topic);
	free(lethread);

	return 0;
}

/*
 * Returns number of lemessages in the given lethread
 */
uint64_t lethread_message_count(struct LeThread *lethread) {
	return lethread->next_message_id - lethread->first_message_id;
}


/*
 * Returns number of participants in the given lethread
 */
uint64_t lethread_participant_count(struct LeThread *lethread) {
	return lethread->next_participant_id - lethread->first_participant_id;
}

/*
 * Creates a new LeMessage and adds it to the given thread.
 */
struct LeMessage * lemessage_create(struct LeThread *lethread, uint64_t author_id, char *text) {
	struct LeMessage *new_lemessage = (struct LeMessage *)malloc(sizeof(struct LeMessage));
	size_t length = strlen(text) + 1;

	new_lemessage->author_id = author_id;
	new_lemessage->id = lethread->next_message_id++;

	new_lemessage->text = malloc(length);
	new_lemessage->text[length - 1] = 0;
	strncpy(new_lemessage->text, text, length);

	queue_push(lethread->messages, new_lemessage, sizeof(struct LeMessage));
	free(new_lemessage);

	return lethread->messages->last->data;
}

/*
 * Safely deletes LeMessage.
 */
int32_t lemessage_delete(struct LeMessage *message) {
	free(message->text);
	free(message);
	return 0;
}

/*
 * Creates a new LeAuthor and adds it to the given thread.
 */
struct LeAuthor * leauthor_create(struct LeThread *lethread) {
	struct LeAuthor *new_leauthor = (struct LeAuthor *)malloc(sizeof(struct LeAuthor));

	new_leauthor->id = lethread->next_participant_id;
	rand_string(new_leauthor->token, sizeof(new_leauthor->token) - 1);

	queue_push(lethread->participants, new_leauthor, sizeof(struct LeAuthor));

	return new_leauthor;
}

/*
 * Safely deletes LeAuthor.
 */
int32_t leauthor_delete(struct LeAuthor *author) {
	free(author);
	return 0;
}

/*
 * Opens one (specified in filename) of the lethread files
 * 
 * If file/directory doesn't exist and create==TRUE, creates file/directory,
 * otherwise returns ERRNSFD.
 */
FILE * get_le_file(uint64_t lethread_id, char *mode, char *filename, int8_t create) {
	char path[256];
	char id_str[32];

	struct stat st = {0};

	memset(id_str, 0, sizeof(id_str));
	memset(path, 0, sizeof(path));

	sprintf(path, DATA_DIR "/%llu/", lethread_id);

	/* Check if the directory exists */
	if (stat(path, &st) == -1) {
		if (!create) return ERRNSFD;
		mkdir(path, 0700);
	}

	strcat(path, filename);

	/* Check if the file exists */
	if (stat(path, &st) == -1 && !create) {
		return ERRNSFD;
	}

	FILE *file = fopen(path, mode);

	return file;
}

/*
 * Saves LeThread to the corresponding file.
 */
int8_t lethread_save(struct LeThread *lethread) {
	size_t topic_length = strlen(lethread->topic);
	FILE *lethread_info_file;

	/* This trick clears the file so we don't have to have a headache with all these overwriting file stuff */
	lethread_info_file = get_le_file(lethread->id, "wb", FILENAME_LETHREAD, TRUE);
	fclose(lethread_info_file);

	lethread_info_file = get_le_file(lethread->id, "ab", FILENAME_LETHREAD, TRUE);

	fwrite(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fwrite(&lethread->author_id, sizeof(lethread->author_id), 1, lethread_info_file);
	fwrite(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fwrite(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fwrite(&lethread->first_participant_id, sizeof(lethread->first_participant_id), 1, lethread_info_file);
	fwrite(&lethread->next_participant_id, sizeof(lethread->next_participant_id), 1, lethread_info_file);
	fwrite(&topic_length, sizeof(topic_length), 1, lethread_info_file);
	fwrite(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);
}

/*
 * Loads LeThread from the corresponding file.
 * 
 * If file is not found, ERRNSFD is returned.
 */
int8_t lethread_load(struct LeThread *lethread, uint64_t lethread_id) {
	size_t topic_length;
	FILE *lethread_info_file = get_le_file(lethread, "rb", FILENAME_LETHREAD, FALSE);

	if (lethread_info_file == ERRNSFD) {
		return ERRNSFD;
	}

	lethread->messages = queue_create();
	lethread->participants = queue_create();

	fread(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fread(&lethread->author_id, sizeof(lethread->author_id), 1, lethread_info_file);
	fread(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fread(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fread(&lethread->first_participant_id, sizeof(lethread->first_participant_id), 1, lethread_info_file);
	fread(&lethread->next_participant_id, sizeof(lethread->next_participant_id), 1, lethread_info_file);
	fread(&topic_length, sizeof(topic_length), 1, lethread_info_file);
	
	lethread->topic = malloc(topic_length + 1);
	memset(lethread->topic, 0, topic_length + 1);
	
	fread(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);

	return 0;
}

/*
 * Saves LeMessage history to the corresponding file.
 */
int8_t lemessages_save(struct LeThread *lethread) {
	struct QueueNode *node = lethread->messages->first;
	struct LeMessage *lemessage;
	size_t text_length;
	FILE *lemessages_file;

	/* This trick clears the file so we don't have to have a headache with all these overwriting file stuff */
	lemessages_file = get_le_file(lethread->id, "wb", FILENAME_LEMESSAGES, TRUE);
	fclose(lemessages_file);

	lemessages_file = get_le_file(lethread->id, "ab", FILENAME_LEMESSAGES, TRUE);

	while (node != NULL) {
		lemessage = node->data;
		text_length = strlen(lemessage->text);
		fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
		fwrite(&lemessage->author_id, sizeof(lemessage->author_id), 1, lemessages_file);
		fwrite(&text_length, sizeof(text_length), 1, lemessages_file);
		fwrite(lemessage->text, 1, text_length, lemessages_file);
		node = node->next;
	}

	fclose(lemessages_file);
}

/*
 * Saves one LeMessage to the corresponding file.
 */
int8_t lemessage_save(struct LeThread *lethread, struct LeMessage *lemessage) {
	size_t text_length = strlen(lemessage->text);
	FILE *lemessages_file;
	
	lemessages_file = get_le_file(lethread->id, "ab", FILENAME_LEMESSAGES, TRUE);
	
	fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
	fwrite(&lemessage->author_id, sizeof(lemessage->author_id), 1, lemessages_file);
	fwrite(&text_length, sizeof(text_length), 1, lemessages_file);
	fwrite(lemessage->text, 1, text_length, lemessages_file);
	
	fclose(lemessages_file);
}

/*
 * Loads LeMessage history from the corresponding file to the lethread object.
 * 
 * If file is not found, ERRNSFD is returned.
 */
int8_t lemessages_load(struct LeThread *lethread) {
	size_t text_length;
	FILE *lemessages_file = get_le_file(lethread->id, "rb", FILENAME_LEMESSAGES, FALSE);
	struct LeMessage lemessage = {0};

	if (lemessages_file == ERRNSFD) {
		return ERRNSFD;
	}

	for (size_t i = 0; i < lethread_message_count(lethread); ++i) {
		fread(&lemessage.id, sizeof(lemessage.id), 1, lemessages_file);
		fread(&lemessage.author_id, sizeof(lemessage.author_id), 1, lemessages_file);
		fread(&text_length, sizeof(text_length), 1, lemessages_file);

		lemessage.text = malloc(text_length + 1);
		memset(lemessage.text, 0, text_length + 1);
		fread(lemessage.text, 1, text_length, lemessages_file);

		queue_push(lethread->messages, &lemessage, sizeof(struct LeMessage));
	}

	fclose(lemessages_file);

	return 0;
}

int8_t lemessage_save(struct LeThread *lethread, struct LeMessage *lemessage) {
	size_t text_length = strlen(lemessage->text);
	FILE *lemessages_file;
	
	lemessages_file = get_le_file(lethread->id, "ab", FILENAME_LEMESSAGES, TRUE);
	
	fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
	fwrite(&lemessage->author_id, sizeof(lemessage->author_id), 1, lemessages_file);
	fwrite(&text_length, sizeof(text_length), 1, lemessages_file);
	fwrite(lemessage->text, 1, text_length, lemessages_file);
	
	fclose(lemessages_file);
}