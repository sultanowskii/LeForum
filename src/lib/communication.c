#include "lib/communication.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>

#include "lib/constants.h"
#include "lib/security.h"
#include "lib/status.h"

ssize_t s_send(int sockfd, void *buf, size_t size, int flags) {
	size_t sent = 0;
	size_t tmp = 0;

	while (sent < size) {
		tmp = send(sockfd, buf + sent, size - sent, flags);

		if (tmp == 0)
			goto SSEND_EXIT;

		if ((ssize_t)tmp < 0)
			goto SSEND_EXIT;

		sent += tmp;
	}

SSEND_EXIT:
	return sent;
}

ssize_t s_recv(int sockfd, void *buf, size_t size, int flags) {
	size_t received = 0;
	size_t tmp = 0;

	while (received < size) {
		tmp = recv(sockfd, buf + received, size - received, flags);

		if (tmp == 0)
			goto SRECV_EXIT;

		if ((ssize_t)tmp < 0)
			goto SRECV_EXIT;
		received += tmp;
	}

SRECV_EXIT:
	return received;
}

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
