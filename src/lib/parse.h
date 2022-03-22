#include <string.h>
#include <inttypes.h>

#include "lib/forum.h"
#include "lib/constants.h"
#include "lib/status.h"

#define CMD_COUNT 2

struct LeCommand {
	char *name;
	void (*parse)(char *, size_t);
};

void cmd_get_lethread(char *raw_data, size_t size);
void cmd_send_lemessage(char *raw_data, size_t size);
status_t query_parse(char *raw_data, size_t size);