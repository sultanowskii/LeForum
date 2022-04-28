#include "lib/communication.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>

#include "lib/constants.h"
#include "lib/security.h"
#include "lib/status.h"

ssize_t sendf(int fd, char *str, ...) {
	va_list args;
	char    buffer[4096];
	int     result        = 0;

	NULLPTR_PREVENT(str, -LESTATUS_NPTR)

	va_start(args, str);

	vsnprintf(buffer, (size_t)4095, str, args);
	result = send(fd, buffer, strlen(buffer), 0);

	va_end(args);

	return result;
}
