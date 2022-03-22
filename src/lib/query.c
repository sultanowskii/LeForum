#include "lib/query.h"

struct LeCommand CMDS[CMD_COUNT] = {
	{"GET", cmd_get_lethread},
	{"MSG", cmd_send_lemessage}
};

/*
 * Parses a query, if valid, retrieves lethread information,
 * including topic, message history, etc.
 */
struct LeCommandResult cmd_get_lethread(char *raw_data, size_t size) {

}

/*
 * Parses a query, if valid, adds a new message to the specific lethread.
 *
 * TOKEN is an optional argument. If not presented/not correct, then the message
 * will be posted anonymously.
 */
struct LeCommandResult cmd_send_lemessage(char *raw_data, size_t size) {
	uint64_t lethread_id;
	uint16_t text_size;
	char *text;
	bool_t is_author;
	char token[TOKEN_LENGTH];
	struct LeThread *lethread;
	struct LeCommandResult result = {0, LESTATUS_OK, NULL};

	char *data_ptr = raw_data;

	if (size < sizeof("THRID") + sizeof(lethread_id) + sizeof("TXTSZ") + sizeof(text_size) + sizeof("TXT")) {
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
		result.status = LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TXTSZ", 5) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 5;

	text_size = *(uint16_t *)data_ptr;

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

	lemessage_create(lethread, text, is_author);
	free(text);
	
	result.status = LESTATUS_OK;
	result.size = 0;
	result.data = NULL;

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