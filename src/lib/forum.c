#include "lib/forum.h"

/*
 * Creates new LeThread, if there is no one with provided id.
 * If there is, then returns ERREXST.
 */
struct LeThread * lethread_create(char *topic, u_int64_t id) {
	// If the lethread file already exists, then nothing should be done
	FILE* lethread_file = lethread_get_file(id, "rb", FALSE);
	if (lethread_file != ERRNSFD) {
		fclose(lethread_file);
		return ERREXST;
	}
	
	struct LeThread *new_lethread = (struct LeThread *)malloc(sizeof(struct LeThread));
	size_t topic_length = strlen(topic);

	new_lethread->id = id;
	new_lethread->author_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->first_message_id = rand_u_int64_t() % 0xffffffff;
	new_lethread->last_message_id = new_lethread->first_message_id;
	new_lethread->first_participant_id = new_lethread->author_id;
	new_lethread->last_participant_id = new_lethread->first_participant_id;
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
 * Creates a new LeMessage and adds it to the given thread.
 */
struct LeMessage * lemessage_create(struct LeThread *lethread, u_int64_t author_id, char *text) {
	struct LeMessage *new_lemessage = (struct LeMessage *)malloc(sizeof(struct LeMessage));
	size_t length = strlen(text) + 1;

	new_lemessage->author_id = author_id;
	new_lemessage->id = ++lethread->last_message_id;

	new_lemessage->text = malloc(length);
	new_lemessage->text[length - 1] = 0;
	strncpy(new_lemessage->text, text, length);

	queue_push(lethread->messages, new_lemessage, sizeof(struct LeMessage));

	return new_lemessage;
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

	new_leauthor->id = ++lethread->last_participant_id;
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
 * Opens lethread (with given id) file with the desired mode.
 * 
 * If file/directory doesn't exist and create==TRUE, creates file/directory,
 * otherwise returns ERRNSFD.
 */
FILE * lethread_get_file(u_int64_t id, char *mode, int8_t create) {
	char filename[256];
	char id_str[32];

	struct stat st = {0};

	memset(id_str, 0, sizeof(id_str));
	memset(filename, 0, sizeof(filename));

	sprintf(filename, DATA_DIR "/%llu/", id);

	/* Check if the directory exists */
	if (stat(filename, &st) == -1) {
		if (!create) return ERRNSFD;
		mkdir(filename, 0700);
	}

	strcat(filename, "" THREAD_FILENAME);

	/* Check if the file exists */
	if (stat(filename, &st) == -1 && !create) {
		return ERRNSFD;
	}

	FILE *lethread_info_file = fopen(filename, mode);

	return lethread_info_file;
}

/*
 * Saves LeThread to the corresponding file.
 */
int8_t lethread_save(struct LeThread *lethread) {
	size_t topic_length = strlen(lethread->topic);
	FILE *lethread_info_file;

	/* This trick clears the file so we don't have to have a headache with all these overwriting file stuff */
	lethread_info_file = lethread_get_file(lethread->id, "wb", TRUE);
	fclose(lethread_info_file);

	lethread_info_file = lethread_get_file(lethread->id, "ab", TRUE);

	fwrite(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fwrite(&lethread->author_id, sizeof(lethread->author_id), 1, lethread_info_file);
	fwrite(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fwrite(&lethread->last_message_id, sizeof(lethread->last_message_id), 1, lethread_info_file);
	fwrite(&lethread->first_participant_id, sizeof(lethread->first_participant_id), 1, lethread_info_file);
	fwrite(&lethread->last_participant_id, sizeof(lethread->last_participant_id), 1, lethread_info_file);
	fwrite(&topic_length, sizeof(topic_length), 1, lethread_info_file);
	fwrite(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);
}

/*
 * Loads LeThread from the corresponding file.
 * 
 * If file is not found, ERRNSFD is returned.
 */
int8_t lethread_load(struct LeThread *lethread, u_int64_t id) {
	size_t topic_length;
	FILE *lethread_info_file = lethread_get_file(id, "rb", FALSE);

	if (lethread_info_file == ERRNSFD) {
		return ERRNSFD;
	}

	lethread->messages = queue_create();
	lethread->participants = queue_create();

	fread(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fread(&lethread->author_id, sizeof(lethread->author_id), 1, lethread_info_file);
	fread(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fread(&lethread->last_message_id, sizeof(lethread->last_message_id), 1, lethread_info_file);
	fread(&lethread->first_participant_id, sizeof(lethread->first_participant_id), 1, lethread_info_file);
	fread(&lethread->last_participant_id, sizeof(lethread->last_participant_id), 1, lethread_info_file);
	fread(&topic_length, sizeof(topic_length), 1, lethread_info_file);
	
	lethread->topic = malloc(topic_length + 1);
	memset(lethread->topic, 0, topic_length + 1);
	
	fread(lethread->topic, 1, topic_length, lethread_info_file);

	fclose(lethread_info_file);
}
