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
	size_t topic_size;
	size_t text_size;
	size_t data_size;
	size_t chunk_size;
	size_t message_cnt;
	char *data;
	char *data_start;

	struct LeCommandResult result = {0, LESTATUS_OK, NULL};
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
	lethread = lethread_get_by_id(lethread_id);

	if (lethread == LESTATUS_NFND) {
		result.status = LESTATUS_NFND;
		return result;
	}

	topic_size = strlen(lethread->topic);

	message_cnt = lethread_message_count(lethread);
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
		data += text_size;

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
	struct LeThread *new_lethread;
	size_t topic_size;
	char *answer_start, *answer;

	struct LeCommandResult result = {0, LESTATUS_OK, NULL};
	char *data_ptr = raw_data;
	size_t answer_size = sizeof("OKTHRID") - 1 + sizeof(uint64_t) + sizeof("TKN") - 1 + TOKEN_SIZE;

	if (size < sizeof("TPCSZ") - 1 + sizeof(topic_size) + sizeof("TPC") - 1) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "TPCSZ", sizeof("TPCSZ") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPCSZ") - 1;

	topic_size = *(size_t *)data_ptr;
	data_ptr += sizeof(size_t);

	if (strncmp(data_ptr, "TPC", sizeof("TPC") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPC") - 1;

	new_lethread = s_lethread_create(data_ptr, rand_uint64_t() % 0xffffffff);
	leauthor_create(new_lethread, TRUE);

	s_lethread_save(new_lethread);
	s_leauthor_save(new_lethread);

	answer_start = malloc(answer_size);
	answer = answer_start;

	strncpy(answer, "OKTHRID", sizeof("OKTHRID") - 1);
	answer += sizeof("OKTHRID") - 1;

	*(uint64_t*)answer = new_lethread->id;
	answer += sizeof(uint64_t);

	strncpy(answer, "TKN", sizeof("TKN") - 1);
	answer += sizeof("TKN") - 1;

	strncpy(answer, new_lethread->author->token, TOKEN_SIZE);
	answer += TOKEN_SIZE;

	result.data = answer_start;
	result.size = answer_size;
	result.status = LESTATUS_OK;

	return result;
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
	struct LeCommandResult result = {0, LESTATUS_OK, NULL};

	char *data_ptr = raw_data;

	if (size < sizeof("THRID") - 1 + sizeof(lethread_id) + sizeof("TXTSZ") - 1 + sizeof(text_size) + sizeof("TXT") - 1) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", sizeof("THRID") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("THRID") - 1;

	lethread_id = *(uint64_t *)data_ptr;
	data_ptr += sizeof(lethread_id);

	lethread = lethread_get_by_id(lethread_id);

	if (lethread == LESTATUS_NFND) {
		result.status = LESTATUS_NFND;
		return result;
	}

	if (strncmp(data_ptr, "TXTSZ", sizeof("TXTSZ") - 1) != 0) {
		free(lethread);
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TXTSZ") - 1;

	text_size = *(size_t *)data_ptr;
	data_ptr += sizeof(text_size);

	if (strncmp(data_ptr, "TXT", 3) != 0) {
		free(lethread);
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 3;

	text = malloc(text_size + 1);
	text[text_size] = '\0';
	strncpy(text, data_ptr, text_size);

	is_author = FALSE;

	if (strncmp(data_ptr, "TKN", sizeof("TXT") - 1) == 0) {
		data_ptr += sizeof("TXT") - 1;
		is_author = is_token_valid(lethread, data_ptr);
	}

	lemessage = lemessage_create(lethread, text, is_author);
	s_lemessage_save(lemessage);
	s_lethread_save(lethread);

	free(text);

	result.status = LESTATUS_OK;
	result.size = 0;
	result.data = NULL;

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
	size_t cmd_name_size;

	for (size_t i = 0; i < CMD_COUNT; i++) {
		if (strncmp(raw_data, CMDS[i].name, strlen(CMDS[i].name)) == 0) {
			cmd = CMDS[i];
			cmd_name_size = strlen(CMDS[i].name);
			break;
		}
	}

	if (cmd.name == NULL || cmd.process == NULL) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	return cmd.process(raw_data + cmd_name_size, size - cmd_name_size);
}