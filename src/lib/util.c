#include "lib/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/status.h"

size_t s_fgets(char *s, size_t n, FILE* fd) {
	if (fgets(s, n, fd) == NULL) {
		/* Here we're preventing infinite loops if CTRL+D (aka EOF) is pressed */
		clearerr(fd);
		return -LESTATUS_CLIB;
	}

	s[strcspn(s, "\n")] = 0;

	return strlen(s);
}

size_t s_fgets_range(char *s, size_t _min, size_t _max, FILE* fd) {
	size_t tmp  = 0;
	size_t size = 0;

	while (size < _min) {
		if ((status_t)(tmp = s_fgets(s + size, _max + 1 - size, fd)) == -LESTATUS_CLIB)
			return -LESTATUS_IDAT;
		size += tmp;
	}

	return size;
}

char *le_strcat(char *dest, const char *src) {
	while (*dest) dest++;
	while ((*dest++ = *src++) != '\0');
	return --dest;
}

char *le_strncat(char *dest, const char *src, size_t n) {
	size_t i;

	while (*dest) dest++;
	for (i = 0; i < n && src[i] != '\0'; i++) {
		*dest++ = src[i];
	}
	*dest = '\0';
	return dest;
}
