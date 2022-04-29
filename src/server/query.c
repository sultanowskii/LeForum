#include "server/query.h"

#include <string.h>
#include <inttypes.h>

#include "lib/constants.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/security.h"
#include "lib/shared_ptr.h"
#include "lib/status.h"
#include "server/conf.h"

LeCommand CMDS[CMD_COUNT] = {
	{"GTHR", cmd_lethread_get},
	{"CTHR", cmd_lethread_create},
	{"FTHR", cmd_lethread_find},
	{"CMSG", cmd_lemessage_create},
	{"META", cmd_meta},
	{"LIVE", cmd_alive}
};

LeCommandResult cmd_lethread_get(char *raw_data, size_t size) {
	char            *data_ptr            = raw_data;
	LeThread        *lethread            = NULL;
	uint64_t         lethread_id;
	SharedPtr       *sharedptr_lethread;
	size_t           topic_size;
	size_t           message_cnt;
	QueueNode       *node;
	LeMessage       *lemessage;
	size_t           text_size;
	size_t           chunk_size;
	char            *response;
	char            *response_start;
	size_t           response_size;
	LeCommandResult  result              = {0, -LESTATUS_OK, NULL};

	if (size < strlen("THRID") + sizeof(lethread_id)) {
		result.status = -LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", strlen("THRID")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("THRID");

	lethread_id = *(uint64_t *)data_ptr;

	sharedptr_lethread = lethread_get_by_id(lethread_id);

	if (sharedptr_lethread == -LESTATUS_NFND) {
		result.status = -LESTATUS_NFND;
		return result;
	}

	lethread = (LeThread *)sharedptr_lethread->data;

	topic_size = strlen(lethread->topic);

	message_cnt = lethread_message_count(lethread);
	response_size = strlen("THRID") + sizeof(lethread_id) + strlen("TPCSZ") + sizeof(topic_size) + strlen("TPC") + topic_size + strlen("MSGCNT") + sizeof(message_cnt);
	chunk_size = response_size + 1024;

	response_start = malloc(chunk_size + 1);
	response = response_start;

	strncpy(response, "THRID", strlen("THRID"));
	response += strlen("THRID");

	*(uint64_t *)response = lethread_id;
	response += sizeof(lethread_id);

	strncpy(response, "TPCSZ", strlen("TPCSZ"));
	response += strlen("TPCSZ");
	*(size_t *)response = topic_size;
	response += sizeof(topic_size);

	strncpy(response, "TPC", strlen("TPC"));
	response += strlen("TPC");

	strncpy(response, lethread->topic, topic_size);
	response += topic_size;

	strncpy(response, "MSGCNT", strlen("MSGCNT"));
	response += strlen("MSGCNT");

	*(uint64_t *)response = message_cnt;
	response += sizeof(message_cnt);

	node = lethread->messages->first;

	while (node != NULL) {
		lemessage = (LeMessage *)node->data;

		text_size = strlen(lemessage->text);

		while (response_size + strlen("MSG") + sizeof(lemessage->by_lethread_author) + sizeof(lemessage->id) + sizeof(text_size) + text_size + strlen("MSGEND") >= chunk_size) {
			chunk_size *= 2;
			response_start = realloc(response_start, chunk_size);
			response = response_start + response_size;
		}

		strncpy(response, "MSG", strlen("MSG"));
		response += strlen("MSG");

		*(uint8_t *)response = lemessage->by_lethread_author;
		response += sizeof(lemessage->by_lethread_author);
	
		*(uint64_t *)response = lemessage->id;
		response += sizeof(lemessage->id);

		*(size_t *)response = text_size;
		response += sizeof(text_size);

		strncpy(response, lemessage->text, text_size);
		response += text_size;

		strncpy(response, "MSGEND", strlen("MSGEND"));
		response += strlen("MSGEND");
		node = node->next;

		response_size = response - response_start;
	}

	sharedptr_delete(sharedptr_lethread);
	sharedptr_lethread = nullptr;

	result.data = response_start;
	result.size = response - response_start;
	result.status = -LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lethread_create(char *raw_data, size_t size) {
	char            *data_ptr            = raw_data;
	LeThread        *new_lethread;
	size_t           topic_size;
	SharedPtr       *sharedptr_lethread;
	char            *response_start;
	char            *response;
	size_t           response_size       = strlen("OKTHRID") + sizeof(new_lethread->id) + strlen("TKN") + TOKEN_SIZE;
	LeCommandResult  result              = {0, -LESTATUS_OK, NULL};

	if (size < strlen("TPCSZ") + sizeof(topic_size) + strlen("TPC")) {
		result.status = -LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "TPCSZ", strlen("TPCSZ")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TPCSZ");

	topic_size = *(size_t *)data_ptr;
	data_ptr += sizeof(topic_size);

	if (topic_size > MAX_TOPIC_SIZE || topic_size < MIN_TOPIC_SIZE) {
		result.status = -LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TPC", strlen("TPC")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TPC");

	sharedptr_lethread = s_lethread_create(data_ptr, rand_uint64_t() % 0xffffffff);
	new_lethread = (LeThread *)sharedptr_lethread->data;

	leauthor_create(new_lethread, TRUE);

	s_lethread_save(sharedptr_lethread);
	s_leauthor_save(sharedptr_lethread);

	response_start = malloc(response_size + 1);
	response = response_start;

	strncpy(response, "THRID", strlen("THRID"));
	response += strlen("THRID");

	*(uint64_t*)response = new_lethread->id;
	response += sizeof(new_lethread->id);

	strncpy(response, "TKN", strlen("TKN"));
	response += strlen("TKN");

	strncpy(response, new_lethread->author->token, TOKEN_SIZE);
	response += TOKEN_SIZE;

	sharedptr_delete(sharedptr_lethread);
	sharedptr_lethread = nullptr;

	result.data = response_start;
	result.size = response_size;
	result.status = -LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lethread_find(char *raw_data, size_t size) {
	char            *data_ptr            = raw_data;
	Queue           *lethreads;
	LeThread        *lethread            = NULL;
	uint64_t         lethread_id;
	SharedPtr       *sharedptr_lethread;
	size_t           topic_size;
	QueueNode       *node;
	char            *topic_part;
	size_t           topic_part_size;
	size_t           chunk_size;
	char            *response;
	char            *response_start;
	size_t           response_size;
	LeCommandResult  result              = {0, -LESTATUS_OK, NULL};

	if (strncmp(data_ptr, "TPCPSZ", strlen("TPCPSZ")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TPCPSZ");

	topic_part_size = *(size_t *)data_ptr;
	data_ptr += sizeof(topic_part_size);

	if (topic_part_size > MAX_TOPIC_SIZE || topic_part_size < MIN_TOPIC_SIZE) {
		result.status = -LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TPCP", strlen("TPCP")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TPCP");

	topic_part = malloc(topic_part_size + 1);
	strncpy(topic_part, data_ptr, topic_part_size);
	topic_part[topic_part_size] = '\0';
	data_ptr += topic_part_size;

	lethreads = lethread_find(topic_part, topic_part_size);

	node = lethreads->first;

	chunk_size = 1024;
	response_start = malloc(chunk_size + 1);
	response = response_start;
	response_size = response - response_start;

	if (queue_is_empty(lethreads) || node == NULL) {
		strncpy(response, "NFND", strlen("NFND"));
		response += strlen("NFND");
		goto FTHR_SUCCESS;
	}

	while (node != NULL) {
		sharedptr_lethread = (SharedPtr *)node->data;
		lethread = (LeThread *)sharedptr_lethread->data;

		topic_size = strlen(lethread->topic);

		while (response_size + strlen("THRID") + sizeof(lethread->id) + strlen("TPCSZ") + sizeof(topic_size) + strlen("TPC") + topic_size >= chunk_size) {
			chunk_size *= 2;
			response_start = realloc(response_start, chunk_size);
			response = response_start + response_size;
		}

		strncpy(response, "THRID", strlen("THRID"));
		response += strlen("THRID");

		*(uint64_t *)response = lethread->id;
		response += sizeof(lethread->id);

		strncpy(response, "TPCSZ", strlen("TPCSZ"));
		response += strlen("TPCSZ");
		*(size_t *)response = topic_size;
		response += sizeof(topic_size);

		strncpy(response, "TPC", strlen("TPC"));
		response += strlen("TPC");

		strncpy(response, lethread->topic, topic_size);
		response += topic_size;

		response_size = response - response_start;

		node = node->next;
	}

FTHR_SUCCESS:
	queue_delete(lethreads);
	lethreads = nullptr;

	free(topic_part);
	topic_part = nullptr;

	result.data = response_start;
	result.size = response - response_start;
	result.status = -LESTATUS_OK;

	return result;
}

LeCommandResult cmd_lemessage_create(char *raw_data, size_t size) {
	char            *data_ptr            = raw_data;
	LeThread        *lethread;
	uint64_t         lethread_id;
	SharedPtr       *sharedptr_lethread;
	LeMessage       *lemessage;
	char            *text;
	size_t           text_size;
	bool_t           is_author;
	LeCommandResult  result              = {0, -LESTATUS_OK, NULL};

	if (size < strlen("THRID") + sizeof(lethread_id) + strlen("TXTSZ") + sizeof(text_size) + strlen("TXT")) {
		result.status = -LESTATUS_ISYN;
		return result;
	}

	if (strncmp(data_ptr, "THRID", strlen("THRID")) != 0) {
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("THRID");

	lethread_id = *(uint64_t *)data_ptr;
	data_ptr += sizeof(lethread_id);

	sharedptr_lethread = lethread_get_by_id(lethread_id);

	if (sharedptr_lethread == -LESTATUS_NFND) {
		result.status = -LESTATUS_NFND;
		return result;
	}

	lethread = (LeThread *)sharedptr_lethread->data;

	if (strncmp(data_ptr, "TXTSZ", strlen("TXTSZ")) != 0) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TXTSZ");

	text_size = *(size_t *)data_ptr;
	data_ptr += sizeof(text_size);

	if (text_size > MAX_MESSAGE_SIZE || text_size < MIN_MESSAGE_SIZE) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = -LESTATUS_IDAT;
		return result;
	}

	if (strncmp(data_ptr, "TXT", strlen("TXT")) != 0) {
		sharedptr_delete(sharedptr_lethread);
		sharedptr_lethread = nullptr;
		result.status = -LESTATUS_ISYN;
		return result;
	}
	data_ptr += strlen("TXT");

	text = malloc(text_size + 1);
	text[text_size] = '\0';
	strncpy(text, data_ptr, text_size);
	data_ptr += text_size;

	is_author = FALSE;

	/* TOKEN is an optional argument. If not presented/not correct, then the message  will be posted anonymously. */
	if (strncmp(data_ptr, "TKN", strlen("TKN")) == 0) {
		data_ptr += strlen("TKN");
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
	result.status = -LESTATUS_OK;

	return result;
}

LeCommandResult cmd_meta(char *raw_data, size_t size) {
	LeCommandResult  result          = {0, -LESTATUS_OK, NULL};
	char            *response;
	char            *response_start;
	const char      *tmp;
	size_t           tmp_size;

	response = malloc(128);
	response_start = response;

	strncpy(response, "MINTPCSZ", strlen("MINTPCSZ"));
	response += strlen("MINTPCSZ");

	*(size_t *)response = MIN_TOPIC_SIZE;
	response += sizeof(size_t);

	strncpy(response, "MAXTPCSZ", strlen("MAXTPCSZ"));
	response += strlen("MAXTPCSZ");

	*(size_t *)response = MAX_TOPIC_SIZE;
	response += sizeof(size_t);

	strncpy(response, "MINMSGSZ", strlen("MINMSGSZ"));
	response += strlen("MINMSGSZ");

	*(size_t *)response = MIN_MESSAGE_SIZE;
	response += sizeof(size_t);

	strncpy(response, "MAXMSGSZ", strlen("MAXMSGSZ"));
	response += strlen("MAXMSGSZ");

	*(size_t *)response = MAX_MESSAGE_SIZE;
	response += sizeof(size_t);

	strncpy(response, "THRN", strlen("THRN"));
	response += strlen("THRN");

	*(size_t *)response = get_lethread_count();
	response += sizeof(size_t);

	tmp = get_version();
	tmp_size = strlen(tmp);

	strncpy(response, "VERSZ", strlen("VERSZ"));
	response += strlen("VERSZ");

	*(size_t *)response = tmp_size;
	response += sizeof(size_t);

	strncpy(response, "VER", strlen("VER"));
	response += strlen("VER");

	strncpy(response, tmp, tmp_size);
	response += tmp_size;

	result.data = response_start;
	result.size = response - response_start;
	result.status = -LESTATUS_OK;

	return result;
}

LeCommandResult cmd_alive(char *raw_data, size_t size) {
	LeCommandResult result = {0, -LESTATUS_OK, NULL};

	return result;
}

LeCommandResult query_process(char *raw_data, size_t size) {
	LeCommand       cmd            = {NULL, NULL};
	LeCommandResult result         = {0, -LESTATUS_OK, NULL};
	size_t          cmd_name_size;

	if (raw_data == nullptr) {
		result.status = -LESTATUS_NPTR;
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
		result.status = -LESTATUS_ISYN;
		return result;
	}

	return cmd.process(raw_data + cmd_name_size, size - cmd_name_size);
}