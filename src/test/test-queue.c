#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/queue.h"
#include "lib/status.h"

void str_delete(void *s) {
	free(s);
}

int main() {
	struct Queue *queue = queue_create();
	char tmp[1024];
	bool_t is_empty;
	char *data;

	memset(tmp, 0, 1024);

	for (size_t i = 0; i < 5; ++i) {
		printf("Element #%llu:\n", i);
		scanf("%s", tmp);
		queue_push(queue, tmp, strlen(tmp) + 1);
	}

	puts("=============");

	size_t cntr = 0;
	while (!queue_is_empty(queue)) {
		is_empty = queue_is_empty(queue);
		data = queue_pop(queue);
		printf("queue_is_empty()=%d, pop(), cntr=%llu: %s\n", is_empty, cntr++, data);
		free(data);
	}

	printf("Finished! queue_is_emtpy()=%llu\n", queue_is_empty(queue));

	puts("Destroying queue...");
	queue_delete(queue, str_delete);

	return 0;
}
