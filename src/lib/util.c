#include "lib/util.h"

size_t s_fgets(char *s, size_t n, FILE* fd) {
	if (fgets(s, n, fd) == NULL) {
		/* Here we're preventing infinite loops if CTRL+D (aka EOF) is pressed */
		clearerr(stdin);
		return LESTATUS_CLIB;
	}
	
	s[strcspn(s, "\n")] = 0;

	return strlen(s);
}