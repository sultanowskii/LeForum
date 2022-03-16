#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>

ssize_t sendf(int32_t fd, char* str, ...);
