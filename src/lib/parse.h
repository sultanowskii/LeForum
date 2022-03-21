#include <string.h>
#include <inttypes.h>

#define CMD_COUNT 2

struct LeCommand {
	char *name;
	void (*parse)(char *);
};

void parse_cmd_get(char *raw_data);
void parse_cmd_msg(char *raw_data);
int32_t query_parse(char *raw_data);