#include "lib/parse.h"

struct LeCommand CMDS[CMD_COUNT] = {
	{"GET", parse_cmd_get},
	{"MSG", parse_cmd_msg}
};

void parse_cmd_get(char *raw_data) {
	
}

void parse_cmd_msg(char *raw_data) {
	
}

int32_t query_parse(char *raw_data) {
	struct LeCommand cmd = {0, 0};

	for (size_t i = 0; i < CMD_COUNT; i++) {
		if (strncmp(raw_data, CMDS[i].name, strlen(CMDS[i].name)) == 0) {
			cmd = CMDS[i];
			break;
		}
	}

	if (cmd.name == NULL || cmd.parse == NULL) {
		return -1;
	}

	cmd.parse(raw_data + strlen(cmd.name));
}