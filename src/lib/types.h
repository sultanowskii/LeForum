#include <sys/socket.h>

#pragma once
#define FALSE 0
#define TRUE 1

struct LeClientInfo {
	int32_t fd;
	socklen_t addr_len;
	struct sockaddr *addr;
};

struct LeMessage {
	u_int64_t id;
	u_int64_t author_id;
	char *text;
	struct LeMessage *next;
};

struct LeAuthor {
	u_int64_t id;
};

struct LeThread {
	u_int64_t id;
	u_int64_t author_id;
	u_int64_t first_message_id;
	u_int64_t last_message_id;
	u_int64_t first_author_id;
	u_int64_t last_author_id;
	char *topic;
	struct LeMessage *first_message;
	struct LeMessage *last_message;
};
