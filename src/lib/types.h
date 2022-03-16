#pragma once

#include <sys/socket.h>

#define FALSE 0
#define TRUE 1

struct LeClientInfo {
	int32_t fd;
	socklen_t addr_len;
	struct sockaddr *addr;
};
