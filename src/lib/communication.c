#include "lib/communication.h"

ssize_t sendf(int32_t fd, char *str, ...) {
	va_list                  args;
	char                     buffer[4096];
	int32_t                  result          = 0;


	NULLPTR_PREVENT(str, LESTATUS_NPTR)

	va_start(args, str);

	vsnprintf(buffer, (uint64_t)4095, str, args);
	result = send(fd, buffer, strlen(buffer), 0);

	va_end(args);

	return result;
}
