#include "lib/security.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/constants.h"
#include "lib/status.h"

#define ALLOWED_SYMBOLS     (0x7e - 0x20)
#define MIN_SYMBOL           0x21

inline char *rand_string(size_t size) {
	FILE          *f;
	unsigned char  tmp;
	char          *str;

	str = malloc(size);

	f = fopen("/dev/urandom", "r");

	for (size_t i = 0; i < size; ++i) {
		tmp = 0;
		while (tmp < MIN_SYMBOL) {
			fread(&tmp, 1, 1, f);
			str[i] = tmp % ALLOWED_SYMBOLS + MIN_SYMBOL;
		}
	}

	fclose(f);

	return str;
}

inline uint64_t rand_uint64_t() {
	uint64_t tmp;

	FILE *f = fopen("/dev/urandom", "r");

	fread((char*)&tmp, sizeof(uint64_t), 1, f);

	fclose(f);

	return tmp;
}
