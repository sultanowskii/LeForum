#include "lib/query.h"

struct LeCommand CMDS[CMD_COUNT] = {
	{"GTHR", cmd_get_lethread},
	{"CTHR", cmd_create_lethread},
	{"CMSG", cmd_create_lemessage},
	{"LIV", cmd_alive}
};

/*
 * Parses a query, if valid, retrieves lethread information,
 * including topic, message history, etc.
 */
struct LeCommandResult cmd_get_lethread(char *raw_data, size_t size) {
	struct LeThread *lethread = NULL;
	struct QueueNode *node;
	struct LeMessage *lemessage;
	uint64_t lethread_id;
	char *data;
	char *data_start;

	struct LeCommandResult result = {0, LESTATUS_OK, NULL};
	size_t topic_size = 0;
	size_t text_size = 0;
	size_t data_size = 0;
	size_t chunk_size = 0;
	size_t message_cnt = 0;
	char *data_ptr = raw_data;

	if (size < sizeof("THRID") - 1 + sizeof(lethread_id)) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", 5) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 5;

	lethread_id = *(uint64_t *)data_ptr;
	lethread = malloc(sizeof(struct LeThread));

	if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
		free(lethread);
		result.status = LESTATUS_NFND;
		return result;
	}

	lemessages_load(lethread);

	topic_size = strlen(lethread->topic);

	message_cnt = lethread->next_message_id - lethread->first_message_id;
	data_size = sizeof("THRID") - 1 + sizeof(lethread_id) + sizeof("TPCSZ") - 1 + sizeof(topic_size) + sizeof("TPC") - 1 + topic_size + sizeof("MSGCNT") - 1 + sizeof(message_cnt);
	chunk_size = data_size + 1024;

	data_start = malloc(chunk_size);
	data = data_start;
	
	strncpy(data, "THRID", sizeof("THRID") - 1);
	data += sizeof("THRID") - 1;

	*(uint64_t *)data = lethread_id;
	data += sizeof(lethread_id);

	strncpy(data, "TPCSZ", sizeof("TPCSZ") - 1);
	data += sizeof("TPCSZ") - 1;
	*(size_t *)data = topic_size;
	data += sizeof(topic_size);

	strncpy(data, "TPC", sizeof("TPC") - 1);
	data += sizeof("TPC") - 1;

	strncpy(data, lethread->topic, topic_size);
	data += topic_size;

	strncpy(data, "MSGCNT", sizeof("MSGCNT") - 1);
	data += sizeof("MSGCNT") - 1;

	*(uint64_t *)data = message_cnt;
	data += sizeof(message_cnt);

	node = lethread->messages->first;

	while (node != NULL) {
		lemessage = (struct LeMessage *)node->data;

		text_size = strlen(lemessage->text);

		while (data_size + sizeof("MSG") - 1 + sizeof(uint8_t) + sizeof(size_t) + text_size + sizeof("MSGEND") - 1 >= chunk_size) {
			chunk_size *= 2;
			data_start = realloc(data_start, chunk_size);
			data = data_start + data_size;
		}

		strncpy(data, "MSG", sizeof("MSG") - 1);
		data += sizeof("MSG") - 1;

		*(uint8_t *)data = lemessage->by_lethread_author;
		data += sizeof(lemessage->by_lethread_author);

		*(size_t *)data = text_size;
		data += sizeof(size_t);

		strncpy(data, lemessage->text, text_size);
		data += topic_size;

		strncpy(data, "MSGEND", sizeof("MSGEND") - 1);
		data += sizeof("MSGEND") - 1;
		node = node->next;
	}

	free(lethread);

	result.data = data_start;
	result.size = data - data_start;
	result.status = LESTATUS_OK;

	return result;
}

/*
 * Creates LeThread with given parameters.
 */
struct LeCommandResult cmd_create_lethread(char *raw_data, size_t size) {

}

/*
 * Parses a query, if valid, adds a new message to the specific lethread.
 *
 * TOKEN is an optional argument. If not presented/not correct, then the message
 * will be posted anonymously.
 */
struct LeCommandResult cmd_create_lemessage(char *raw_data, size_t size) {
	struct LeThread *lethread;
	struct LeMessage *lemessage;
	uint64_t lethread_id;
	size_t text_size;
	char *text;
	bool_t is_author;
	char token[TOKEN_SIZE];
	struct LeCommandResult result = {0, LESTATUS_OK, NULL};

	char *data_ptr = raw_data;

	if (size < sizeof("THRID") - 1 + sizeof(lethread_id) + sizeof("TXTSZ") - 1 + sizeof(text_size) + sizeof("TXT") - 1) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", 5) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 5;

	lethread_id = *(uint64_t *)data_ptr;
	data_ptr += sizeof(lethread_id);

	lethread = malloc(sizeof(struct LeThread));
	if (lethread_load(lethread, lethread_id) != LESTATUS_OK) {
		free(lethread);
		result.status = LESTATUS_NFND;
		return result;
	}

	if (strncmp(data_ptr, "TXTSZ", 5) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 5;

	text_size = *(size_t *)data_ptr;
	data_ptr += sizeof(text_size);

	if (strncmp(data_ptr, "TXT", 3) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 3;

	text = malloc(text_size + 1);
	text[text_size] = '\0';
	strncpy(text, data_ptr, text_size);

	is_author = FALSE;

	if (strncmp(data_ptr, "TKN", 3) == 0) {
		data_ptr += 3;
		is_author = is_token_valid(lethread, token);
	}

	lemessage = lemessage_create(lethread, text, is_author);
	lemessage_save(lethread, lemessage);
	free(text);
	
	result.status = LESTATUS_OK;
	result.size = 0;
	result.data = NULL;

	free(lethread);

	return result;
}

/* 
 * Assures the connection is not lost.
 */
struct LeCommandResult cmd_alive(char *raw_data, size_t size) {
	struct LeCommandResult result = {0, LESTATUS_OK, NULL};
	return result;
}

/*
 * Tries to find specified command. If found, executes it.
 */
struct LeCommandResult query_process(char *raw_data, size_t size) {
	struct LeCommand cmd = {NULL, NULL};
	struct LeCommandResult result = {0, LESTATUS_OK, NULL};

	for (size_t i = 0; i < CMD_COUNT; i++) {
		if (strncmp(raw_data, CMDS[i].name, strlen(CMDS[i].name)) == 0) {
			cmd = CMDS[i];
			break;
		}
	}

	if (cmd.name == NULL || cmd.process == NULL) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	return cmd.process(raw_data + strlen(cmd.name), size - strlen(cmd.name));
}