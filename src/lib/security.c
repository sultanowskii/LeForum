#include "lib/security.h"

#define ALLOWED_SYMBOLS (0x7e - 0x20)
#define MIN_SYMBOL 0x21

/*
 * Generates random string of a size=length,
 * puts it into str.
 */
void rand_string(char *str, size_t length) {
	unsigned char tmp;
	FILE *f = fopen("/dev/urandom", "r");
	for (size_t i = 0; i < length; ++i) {
		tmp = 0;
		while (tmp < MIN_SYMBOL) {
			fread(&tmp, 1, 1, f);
			str[i] = tmp % ALLOWED_SYMBOLS + MIN_SYMBOL;
		}
	}
	fclose(f);
}

/*
 * Returns random unsigned 8-byte long number.
 */
u_int64_t rand_u_int64_t() {
	u_int64_t tmp;
	FILE *f = fopen("/dev/urandom", "r");
	fread((char*)&tmp, sizeof(u_int64_t), 1, f);
	fclose(f);
	return tmp;
}
