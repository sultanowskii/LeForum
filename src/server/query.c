#include "server/query.h"

LeCommand CMDS[CMD_COUNT] = {
	{"GTHR", cmd_lethread_get},
	{"CTHR", cmd_lethread_create},
	{"FTHR", cmd_lethread_find},
	{"CMSG", cmd_lemessage_create},
	{"META", cmd_meta},
	{"LIVE", cmd_alive}
};

LeCommandResult cmd_lethread_get(char *raw_data, size_t size) {
	char               *data_ptr       = raw_data;

	LeThread           *lethread       = NULL;
	uint64_t            lethread_id;
	SharedPtr          *sharedptr_lethread;
	size_t              topic_size;
	size_t              message_cnt;

	QueueNode          *node;

	LeMessage          *lemessage;
	size_t              text_size;

	size_t              chunk_size;

	char               *answer;
	char               *answer_start;
	size_t              answer_size;

	LeCommandResult     result         = {0, LESTATUS_OK, NULL};


	if (size < sizeof("THRID") - 1 + sizeof(lethread_id)) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", sizeof("THRID") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("THRID") - 1;

	lethread_id = *(uint64_t *)data_ptr;

	sharedptr_lethread = lethread_get_by_id(lethread_id);

	if (sharedptr_lethread == LESTATUS_NFND) {
		result.status = LESTATUS_NFND;
		return result;
	}

	lethread = (LeThread *)sharedptr_lethread->data;

	topic_size = strlen(lethread->topic);

	message_cnt = lethread_message_count(lethread);
	answer_size = sizeof("THRID") - 1 + sizeof(lethread_id) + sizeof("TPCSZ") - 1 + sizeof(topic_size) + sizeof("TPC") - 1 + topic_size + sizeof("MSGCNT") - 1 + sizeof(message_cnt);
	chunk_size = answer_size + 1024;

	answer_start = malloc(chunk_size);
	answer = answer_start;

	strncpy(answer, "THRID", sizeof("THRID") - 1);
	answer += sizeof("THRID") - 1;

	*(uint64_t *)answer = lethread_id;
	answer += sizeof(lethread_id);

	strncpy(answer, "TPCSZ", sizeof("TPCSZ") - 1);
	answer += sizeof("TPCSZ") - 1;
	*(size_t *)answer = topic_size;
	answer += sizeof(topic_size);

	strncpy(answer, "TPC", sizeof("TPC") - 1);
	answer += sizeof("TPC") - 1;

	strncpy(answer, lethread->topic, topic_size);
	answer += topic_size;

	strncpy(answer, "MSGCNT", sizeof("MSGCNT") - 1);
	answer += sizeof("MSGCNT") - 1;

	*(uint64_t *)answer = message_cnt;
	answer += sizeof(message_cnt);

	node = lethread->messages->first;

	while (node != NULL) {
		lemessage = (LeMessage *)node->data;

		text_size = strlen(lemessage->text);

		while (answer_size + sizeof("MSG") - 1 + sizeof(uint8_t) + sizeof(size_t) + text_size + sizeof("MSGEND") - 1 >= chunk_size) {
			chunk_size *= 2;
			answer_start = realloc(answer_start, chunk_size);
			answer = answer_start + answer_size;
		}

		strncpy(answer, "MSG", sizeof("MSG") - 1);
		answer += sizeof("MSG") - 1;

		*(uint8_t *)answer = lemessage->by_lethread_author;
		answer += sizeof(lemessage->by_lethread_author);
	
		*(uint64_t *)answer = lemessage->id;
		answer += sizeof(uint64_t);

		*(size_t *)answer = text_size;
		answer += sizeof(size_t);

		strncpy(answer, lemessage->text, text_size);
		answer += text_size;

		strncpy(answer, "MSGEND", sizeof("MSGEND") - 1);
		answer += sizeof("MSGEND") - 1;
		node = node->next;

		answer_size = answer - answer_start;
	}

	sharedptr_delete(sharedptr_lethread);
	sharedptr_lethread = nullptr;

	result.data = answer_start;
	result.size = answer - answer_start;
	result.status = LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lethread_create(char *raw_data, size_t size) {
	char               *data_ptr              = raw_data;

	LeThread           *new_lethread;
	size_t              topic_size;
	SharedPtr          *sharedptr_lethread;

	char               *answer_start;
	char               *answer;
	size_t              answer_size           = sizeof("OKTHRID") - 1 + sizeof(uint64_t) + sizeof("TKN") - 1 + TOKEN_SIZE;

	LeCommandResult     result                = {0, LESTATUS_OK, NULL};


	if (size < sizeof("TPCSZ") - 1 + sizeof(topic_size) + sizeof("TPC") - 1) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "TPCSZ", sizeof("TPCSZ") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPCSZ") - 1;

	topic_size = *(size_t *)data_ptr;
	data_ptr += sizeof(size_t);

	if (topic_size > MAX_TOPIC_SIZE || topic_size < MIN_TOPIC_SIZE) {
		result.status = LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TPC", sizeof("TPC") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPC") - 1;

	sharedptr_lethread = s_lethread_create(data_ptr, rand_uint64_t() % 0xffffffff);
	new_lethread = (LeThread *)sharedptr_lethread->data;

	leauthor_create(new_lethread, TRUE);

	s_lethread_save(sharedptr_lethread);
	s_leauthor_save(sharedptr_lethread);

	answer_start = malloc(answer_size);
	answer = answer_start;

	strncpy(answer, "THRID", sizeof("THRID") - 1);
	answer += sizeof("THRID") - 1;

	*(uint64_t*)answer = new_lethread->id;
	answer += sizeof(uint64_t);

	strncpy(answer, "TKN", sizeof("TKN") - 1);
	answer += sizeof("TKN") - 1;

	strncpy(answer, new_lethread->author->token, TOKEN_SIZE);
	answer += TOKEN_SIZE;

	sharedptr_delete(sharedptr_lethread);
	sharedptr_lethread = nullptr;

	result.data = answer_start;
	result.size = answer_size;
	result.status = LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lethread_find(char *raw_data, size_t size) {
	char               *data_ptr       = raw_data;

	Queue              *lethreads;
	LeThread           *lethread       = NULL;
	uint64_t            lethread_id;
	SharedPtr          *sharedptr_lethread;
	size_t              topic_size;

	QueueNode          *node;

	char               *topic_part;
	size_t              topic_part_size;

	size_t              chunk_size;

	char               *answer;
	char               *answer_start;
	size_t              answer_size;

	LeCommandResult     result         = {0, LESTATUS_OK, NULL};


	if (strncmp(data_ptr, "TPCPSZ", sizeof("TPCPSZ") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPCPSZ") - 1;

	topic_part_size = *(size_t *)data_ptr;
	data_ptr += sizeof(topic_part_size);

	if (topic_part_size > MAX_TOPIC_SIZE || topic_part_size < MIN_TOPIC_SIZE) {
		result.status = LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TPCP", sizeof("TPCP") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TPCP") - 1;

	topic_part = malloc(topic_part_size + 1);
	strncpy(topic_part, data_ptr, topic_part_size);
	topic_part[topic_part_size] = '\0';
	data_ptr += topic_part_size;

	lethreads = lethread_find(topic_part, topic_part_size);

	node = lethreads->first;

	chunk_size = 1024;
	answer_start = malloc(chunk_size);
	answer = answer_start;
	answer_size = answer - answer_start;

	if (queue_is_empty(lethreads) || node == NULL) {
		strncpy(answer, "NFND", sizeof("NFND") - 1);
		answer += sizeof("NFND") - 1;
		goto FTHR_SUCCESS;
	}

	while (node != NULL) {
		sharedptr_lethread = (SharedPtr *)node->data;
		lethread = (LeThread *)sharedptr_lethread->data;

		topic_size = strlen(lethread->topic);

		while (answer_size + sizeof("THRID") - 1 + sizeof(lethread->id) + sizeof("TPCSZ") - 1 + sizeof(topic_size) + sizeof("TPC") - 1 + topic_size >= chunk_size) {
			chunk_size *= 2;
			answer_start = realloc(answer_start, chunk_size);
			answer = answer_start + answer_size;
		}

		strncpy(answer, "THRID", sizeof("THRID") - 1);
		answer += sizeof("THRID") - 1;

		*(uint64_t *)answer = lethread->id;
		answer += sizeof(lethread->id);

		strncpy(answer, "TPCSZ", sizeof("TPCSZ") - 1);
		answer += sizeof("TPCSZ") - 1;
		*(size_t *)answer = topic_size;
		answer += sizeof(topic_size);

		strncpy(answer, "TPC", sizeof("TPC") - 1);
		answer += sizeof("TPC") - 1;

		strncpy(answer, lethread->topic, topic_size);
		answer += topic_size;

		answer_size = answer - answer_start;

		node = node->next;
	}

FTHR_SUCCESS:
	queue_delete(lethreads);
	lethreads = nullptr;

	free(topic_part);
	topic_part = nullptr;

	result.data = answer_start;
	result.size = answer - answer_start;
	result.status = LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lemessage_create(char *raw_data, size_t size) {
	char               *data_ptr       = raw_data;

	LeThread           *lethread;
	uint64_t            lethread_id;
	SharedPtr          *sharedptr_lethread;

	LeMessage          *lemessage;
	char               *text;
	size_t              text_size;
	bool_t              is_author;

	LeCommandResult     result         = {0, LESTATUS_OK, NULL};


	if (size < sizeof("THRID") - 1 + sizeof(lethread_id) + sizeof("TXTSZ") - 1 + sizeof(text_size) + sizeof("TXT") - 1) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", sizeof("THRID") - 1) != 0) {
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("THRID") - 1;

	lethread_id = *(uint64_t *)data_ptr;
	data_ptr += sizeof(lethread_id);

	sharedptr_lethread = lethread_get_by_id(lethread_id);

	if (sharedptr_lethread == LESTATUS_NFND) {
		result.status = LESTATUS_NFND;
		return result;
	}

	lethread = (LeThread *)sharedptr_lethread->data;

	if (strncmp(data_ptr, "TXTSZ", sizeof("TXTSZ") - 1) != 0) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += sizeof("TXTSZ") - 1;

	text_size = *(size_t *)data_ptr;
	data_ptr += sizeof(text_size);

	if (text_size > MAX_MESSAGE_SIZE || text_size < MIN_MESSAGE_SIZE) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TXT", 3) != 0) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = LESTATUS_ISYN;
		return result;
	}
	data_ptr += 3;

	text = malloc(text_size + 1);
	text[text_size] = '\0';
	strncpy(text, data_ptr, text_size);

	is_author = FALSE;

	/* TOKEN is an optional argument. If not presented/not correct, then the message  will be posted anonymously. */
	if (strncmp(data_ptr, "TKN", sizeof("TXT") - 1) == 0) {
		data_ptr += sizeof("TXT") - 1;
		is_author = is_token_valid(lethread, data_ptr);
	}

	lemessage = lemessage_create(lethread, text, is_author);
	s_lemessage_save(lemessage);
	s_lethread_save(sharedptr_lethread);

	free(text);
	text = nullptr;

	sharedptr_delete(sharedptr_lethread);
	sharedptr_lethread = nullptr;

	result.data = NULL;
	result.size = 0;
	result.status = LESTATUS_OK;

	return result;
}

LeCommandResult cmd_meta(char *raw_data, size_t size) {
	LeCommandResult     result         = {0, LESTATUS_OK, NULL};

	char               *answer;
	char               *answer_start;

	char               *tmp;
	size_t              tmp_size;


	answer = malloc(128);
	answer_start = answer;

	strncpy(answer, "MINTPCSZ", sizeof("MINTPCSZ") - 1);
	answer += sizeof("MINTPCSZ") - 1;

	*(size_t *)answer = MIN_TOPIC_SIZE;
	answer += sizeof(size_t);

	strncpy(answer, "MAXTPCSZ", sizeof("MAXTPCSZ") - 1);
	answer += sizeof("MAXTPCSZ") - 1;

	*(size_t *)answer = MAX_TOPIC_SIZE;
	answer += sizeof(size_t);

	strncpy(answer, "MINMSGSZ", sizeof("MINMSGSZ") - 1);
	answer += sizeof("MINMSGSZ") - 1;

	*(size_t *)answer = MIN_MESSAGE_SIZE;
	answer += sizeof(size_t);

	strncpy(answer, "MAXMSGSZ", sizeof("MAXMSGSZ") - 1);
	answer += sizeof("MAXMSGSZ") - 1;

	*(size_t *)answer = MAX_MESSAGE_SIZE;
	answer += sizeof(size_t);

	strncpy(answer, "THRN", sizeof("THRN") - 1);
	answer += sizeof("THRN") - 1;

	*(size_t *)answer = get_lethread_number();
	answer += sizeof(size_t);

	tmp = get_version();
	tmp_size = strlen(tmp);

	strncpy(answer, "VERSZ", sizeof("VERSZ") - 1);
	answer += sizeof("VERSZ") - 1;

	*(size_t *)answer = tmp_size;
	answer += sizeof(size_t);

	strncpy(answer, "VER", sizeof("VER") - 1);
	answer += sizeof("VER") - 1;

	strncpy(answer, tmp, tmp_size);
	answer += tmp_size;

	result.data = answer_start;
	result.size = answer - answer_start;
	result.status = LESTATUS_OK;

	return result;
}

LeCommandResult cmd_alive(char *raw_data, size_t size) {
	LeCommandResult     result         = {0, LESTATUS_OK, NULL};

	return result;
}

LeCommandResult query_process(char *raw_data, size_t size) {
	LeCommand           cmd            = {NULL, NULL};
	LeCommandResult     result         = {0, LESTATUS_OK, NULL};
	size_t              cmd_name_size;


	if (raw_data == nullptr) {
		result.status = LESTATUS_NPTR;
		return result;
	}

	for (size_t i = 0; i < CMD_COUNT; i++) {
		if (strncmp(raw_data, CMDS[i].name, strlen(CMDS[i].name)) == 0) {
			cmd = CMDS[i];
			cmd_name_size = strlen(CMDS[i].name);
			break;
		}
	}

	if (cmd.name == nullptr || cmd.process == nullptr) {
		result.status = LESTATUS_ISYN;
		return result;
	}

	return cmd.process(raw_data + cmd_name_size, size - cmd_name_size);
}