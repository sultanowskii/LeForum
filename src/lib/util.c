#include "lib/util.h"

size_t s_fgets(char *s, size_t n, FILE* fd) {
	if (fgets(s, n, fd) == NULL) {
		/* Here we're preventing infinite loops if CTRL+D (aka EOF) is pressed */
		clearerr(stdin);
		return -LESTATUS_CLIB;
	}
	
	s[strcspn(s, "\n")] = 0;

	return strlen(s);
}

size_t s_fgets_range(char *s, size_t _min, size_t _max, FILE* fd) {
	size_t tmp = 0;
	size_t size = 0;

	while (size < _min) {
		if ((tmp = s_fgets(s + size, _max + 1 - size, stdin)) == -LESTATUS_CLIB) {
			return -LESTATUS_IDAT;
		}
		size += tmp;
	}

	return size;
}