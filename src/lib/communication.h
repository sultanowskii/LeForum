#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>

/*
 * Does the same thing as printf() except it
 * prints the result to the socket.
 */
ssize_t sendf(int32_t fd, char *str, ...);
