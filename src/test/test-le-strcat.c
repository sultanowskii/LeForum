#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/util.h"

bool test(char *start, char *end, const char *check);

inline bool test(char *start, char *end, const char *check) {
	if (strlen(start) != (size_t)(end - start) || strcmp(start, check)) {
		return false;
	}
	return true;
}

int main() {
	char *start;
	char *end;
	char *expected;
	int test_n = 0;

	start = calloc(sizeof(char), 1000);

	strcpy(start, "nice");

	test_n++;
	end = le_strcat(start, "_cool");
	expected = "nice_cool";
	if (!test(start, end, expected)) {
		goto UNEXPECTED;
	}

	test_n++;
	end = le_strncat(end, "_awesome", strlen("_awesome"));
	expected = "nice_cool_awesome";
	if (!test(start, end, expected)) {
		goto UNEXPECTED;
	}
	
	test_n++;
	end = le_strncat(end, "_greatDEAD", strlen("_great"));	
	expected = "nice_cool_awesome_great";
	if (!test(start, end, expected)) {
		goto UNEXPECTED;
	}

	free(start);
	puts("Success!");
	return 0;
UNEXPECTED:
	printf("Something went wrong on test #%d\n", test_n);
	printf("Expected: '%s' (size=%zu)\n", expected, strlen(expected));
	printf("Got: '%s' (size=%zu)\n", start, strlen(expected));
	free(start);
	return 1;

}
