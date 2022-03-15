#pragma once
#include <sys/socket.h>

ssize_t sendf(int32_t fd, char* str, ...);