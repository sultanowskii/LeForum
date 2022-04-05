#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/queue.h"
#include "lib/status.h"

void str_delete(void *s) {
	free(s);
}

status_t main() {
	struct Queue            *queue          = queue_create(str_delete);
	char                    *data;

	char                    *tmp;
	bool_t                   is_empty;
	size_t                   cntr           = 0;


	for (size_t i = 0; i < 5; ++i) {
		tmp = malloc(1024);
		printf("Element #%llu:\n", i);
		scanf("%s", tmp);
		queue_push(queue, tmp, strlen(tmp) + 1);
	}

	puts("=============");


	while (!queue_is_empty(queue)) {
		is_empty = queue_is_empty(queue);
		data = queue_pop(queue);
		printf("queue_is_empty()=%d, pop(), cntr=%llu: %s\n", is_empty, cntr++, data);
		free(data);
	}

	printf("Finished! queue_is_emtpy()=%llu\n", queue_is_empty(queue));

	puts("Destroying queue...");

	queue_delete(queue);

	return LESTATUS_OK;
}
