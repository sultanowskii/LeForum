#include "lib/parse.h"

struct LeCommand CMDS[CMD_COUNT] = {
	{"GET", cmd_get_lethread},
	{"MSG", cmd_send_lemessage}
};

/*
 * Retrieves lethread information, including topic, message history, etc.
 */
status_t cmd_get_lethread(char *raw_data, size_t size) {

}

/*
 * Adds a new message to the specific lethread.
 *
 * TOKEN is an optional argument. If not presented/not correct, then the message
 * will be posted anonymously.
 */
status_t cmd_send_lemessage(char *raw_data, size_t size) {
	uint64_t lethread_id;
	uint16_t text_size;
	char *text;
	bool_t is_author;
	char token[TOKEN_LENGTH];
	struct LeThread *lethread;

	char *data_ptr = raw_data;

	if (size < sizeof("THRID") + sizeof(lethread_id) + sizeof("TXTSZ") + sizeof(text_size) + sizeof("TXT")) {
		return LESTATUS_ISYN;
	}

	if (strncmp(data_ptr, "THRID", 5) != 0) {
		return LESTATUS_ISYN;
	}
	data_ptr += 5;

	lethread_id = *(uint64_t *)data_ptr;
	lethread = lethread_get_by_id(lethread_id);

	if (lethread == LESTATUS_NFND) {
		return LESTATUS_IDAT;
	}

	if (strncmp(data_ptr, "TXTSZ", 5) != 0) {
		return LESTATUS_ISYN;
	}
	data_ptr += 5;

	text_size = *(uint16_t *)data_ptr;

	if (strncmp(data_ptr, "TXT", 3) != 0) {
		return LESTATUS_ISYN;
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
}

status_t query_parse(char *raw_data, size_t size) {
	struct LeCommand cmd = {0, 0};

	for (size_t i = 0; i < CMD_COUNT; i++) {
		if (strncmp(raw_data, CMDS[i].name, strlen(CMDS[i].name)) == 0) {
			cmd = CMDS[i];
			break;
		}
	}

	if (cmd.name == NULL || cmd.parse == NULL) {
		return LESTATUS_ISYN;
	}

	cmd.parse(raw_data + strlen(cmd.name), size - strlen(cmd.name));
}