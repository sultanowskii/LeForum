#include "lib/security.h"

#define ALLOWED_SYMBOLS     (0x7e - 0x20)
#define MIN_SYMBOL           0x21

inline status_t rand_string(char *str, size_t size) {
	FILE *              f;
	unsigned char       tmp;


	NULLPTR_PREVENT(str, -LESTATUS_NPTR)

	f = fopen("/dev/urandom", "r");
	for (size_t i = 0; i < size; ++i) {
		tmp = 0;
		while (tmp < MIN_SYMBOL) {
			fread(&tmp, 1, 1, f);
			str[i] = tmp % ALLOWED_SYMBOLS + MIN_SYMBOL;
		}
	}

	fclose(f);

	return -LESTATUS_OK;
}

inline uint64_t rand_uint64_t() {
	uint64_t            tmp;


	FILE *f = fopen("/dev/urandom", "r");

	fread((char*)&tmp, sizeof(uint64_t), 1, f);

	fclose(f);

	return tmp;
}
