#include "lib/util.h"

size_t s_fgets(char *s, size_t n, FILE* fd) {
	size_t    size;

	if (fgets(s, n - 1, fd) == NULL) {
		/* Here we're preventing infinite loops if CTRL+D (aka EOF) is pressed */
		clearerr(stdin);
		return LESTATUS_CLIB;
	}

	size = strlen(s);
	if (size >= n)
		size = n - 1;

	s[size - 1] = '\0';

	return size;
}