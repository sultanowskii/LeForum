#include "client/query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/security.h"

ServerQuery * query_create(void * (*parser)(char *raw_data), char *request_data, size_t size) {
	ServerQuery        *query;


	query = malloc(sizeof(*query));
	query->completed = FALSE;
	query->parse_response = parser;
	query->raw_request_data = request_data;
	query->raw_request_data_size = size;
	query->parsed_data = nullptr;

	return query;
}

status_t query_delete(ServerQuery *query) {
	NULLPTR_PREVENT(query, -LESTATUS_NPTR)

	if (query->raw_request_data != nullptr) {
		free(query->raw_request_data);
		query->raw_request_data = nullptr;
	}

	free(query);

	return -LESTATUS_OK;
}

LeData gen_query_CTHR(const char *topic, size_t size) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = strlen("CTHR") + strlen("TPCSZ") + sizeof(size) + strlen("TPC") + size;

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "CTHR", strlen("CTHR"));
	data_ptr += strlen("CTHR");

	strncpy(data_ptr, "TPCSZ", strlen("TPCSZ"));
	data_ptr += strlen("TPCSZ");

	*(size_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TPC", strlen("TPC"));
	data_ptr += strlen("TPC");

	strncpy(data_ptr, topic, size);
	data_ptr += size;

	return result;
}

LeData gen_query_GTHR(uint64_t thread_id) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = strlen("GTHR") + strlen("THRID") + sizeof(thread_id);

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "GTHR", strlen("GTHR"));
	data_ptr += strlen("GTHR");

	strncpy(data_ptr, "THRID", strlen("THRID"));
	data_ptr += strlen("THRID");

	*(uint64_t *)data_ptr = thread_id;
	data_ptr += sizeof(thread_id);

	return result;
}

LeData gen_query_FTHR(const char *topic_part, size_t size) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = strlen("FTHR") + strlen("TPCPSZ") + sizeof(size) + strlen("TPCP") + size;

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "FTHR", strlen("FTHR"));
	data_ptr += strlen("FTHR");

	strncpy(data_ptr, "TPCPSZ", strlen("TPCPSZ"));
	data_ptr += strlen("TPCPSZ");

	*(size_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TPCP", strlen("TPCP"));
	data_ptr += strlen("TPCP");

	strncpy(data_ptr, topic_part, size);
	data_ptr += size;

	return result;
}

LeData gen_query_CMSG(uint64_t thread_id, const char *msg, size_t size, char *token) {
	LeData result;
	char *data_ptr;
	size_t data_size;

	data_size = strlen("CMSG") + strlen("THRID") + sizeof(thread_id) + strlen("TXTSZ") + sizeof(size) + strlen("TXT") + size;
	if (token != nullptr)
		data_size += strlen("TKN") + TOKEN_SIZE;

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "CMSG", strlen("CMSG"));
	data_ptr += strlen("CMSG");

	strncpy(data_ptr, "THRID", strlen("THRID"));
	data_ptr += strlen("THRID");

	*(uint64_t *)data_ptr = thread_id;
	data_ptr += sizeof(thread_id);

	strncpy(data_ptr, "TXTSZ", strlen("TXTSZ"));
	data_ptr += strlen("TXTSZ");

	*(uint64_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TXT", strlen("TXT"));
	data_ptr += strlen("TXT");

	strncpy(data_ptr, msg, size);
	data_ptr += size;

	if (token != nullptr) {
		strncpy(data_ptr, "TKN", strlen("TKN"));
		data_ptr += strlen("TKN");

		strncpy(data_ptr, token, TOKEN_SIZE);
		data_ptr += TOKEN_SIZE;
	}

	return result;
}

LeData gen_query_META() {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = strlen("META");

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "META", strlen("META"));
	data_ptr += strlen("META");

	return result;
}

LeData gen_query_LIVE() {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = strlen("LIVE");

	result.data = calloc(sizeof(char), data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "LIVE", strlen("LIVE"));
	data_ptr += strlen("LIVE");

	return result;
}

CreatedThreadInfo * parse_response_CTHR(char *raw_data, size_t size) {
	CreatedThreadInfo *info;
	char *data_ptr;
	uint64_t tmp;


	/* TODO: check size */
	data_ptr = raw_data;

	if (strncmp(data_ptr, "THRID", strlen("THRID")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("THRID");

	tmp = *(uint64_t *)data_ptr;
	data_ptr += sizeof(tmp);

	if (strncmp(data_ptr, "TKN", strlen("TKN")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("TKN");

	info = (CreatedThreadInfo *)malloc(sizeof(*info));
	info->thread_id = tmp;
	info->token = malloc(TOKEN_SIZE + 1);
	strncpy(info->token, data_ptr, TOKEN_SIZE);
	info->token[TOKEN_SIZE] = '\0';
	
	return info;
}

LeThread * parse_response_GTHR(char *raw_data, size_t size) {
	char *data_ptr;
	LeThread *thread;
	size_t thread_id;
	char *thread_topic;
	size_t topic_size;
	size_t msg_cntr;
	bool_t by_author;
	uint64_t msg_id;
	size_t msg_size;
	char *msg_text;


	/* TODO: check size */
	data_ptr = raw_data;

	if (strncmp(data_ptr, "THRID", strlen("THRID")) != 0)
		return -LESTATUS_IDAT;
	data_ptr += strlen("THRID");

	thread_id = *(uint64_t *)data_ptr;
	data_ptr += sizeof(thread_id);

	if (strncmp(data_ptr, "TPCSZ", strlen("TPCSZ")) != 0)
		return -LESTATUS_IDAT;
	data_ptr += strlen("TPCSZ");

	topic_size = *(uint64_t *)data_ptr;
	data_ptr += sizeof(topic_size);

	thread_topic = malloc(topic_size + 1);
	if (strncmp(data_ptr, "TPC", strlen("TPC")) != 0) {
		free(thread_topic);
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("TPC");

	strncpy(thread_topic, data_ptr, topic_size);
	thread_topic[topic_size] = '\0';
	data_ptr += topic_size;

	thread = lethread_create(thread_topic, thread_id);

	if (strncmp(data_ptr, "MSGCNT", strlen("MSGCNT")) != 0) {
		lethread_delete(thread);
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("MSGCNT");

	msg_cntr = *(uint64_t *)data_ptr;
	data_ptr += sizeof(msg_cntr);

	for (size_t i = 0; i < msg_cntr; i++) {
		if (strncmp(data_ptr, "MSG", strlen("MSG")) != 0) {
			lethread_delete(thread);
			return -LESTATUS_IDAT;
		}
		data_ptr += strlen("MSG");

		by_author = *(bool_t *)data_ptr;
		data_ptr += sizeof(by_author);

		msg_id = *(uint64_t *)data_ptr;
		data_ptr += sizeof(msg_id);

		msg_size = *(uint64_t *)data_ptr;
		data_ptr += sizeof(msg_size);

		msg_text = malloc(msg_size + 1);
		strncpy(msg_text, data_ptr, msg_size);
		msg_text[msg_size] = '\0';
		data_ptr += msg_size;

		lemessage_create(thread, msg_text, by_author);

		if (strncmp(data_ptr, "MSGEND", strlen("MSGEND")) != 0) {
			lethread_delete(thread);
			return -LESTATUS_IDAT;
		}
		data_ptr += strlen("MSGEND");
	}
	
	return thread;
}

Queue * parse_response_FTHR(char *raw_data, size_t size) {
	char *data_ptr;
	Queue *found;

	LeThread *thread;
	uint64_t thread_id;
	size_t topic_size;
	char *thread_topic;


	found = queue_create(lethread_delete);
	data_ptr = raw_data;

	if (strncmp(data_ptr, "NFND", strlen("NFND")) == 0) {
		return found;
	}

	while ((size_t)(data_ptr - raw_data) < size) {
		if (strncmp(data_ptr, "THRID", strlen("THRID")) != 0) {
			query_delete(found);
			return -LESTATUS_IDAT;
		}
		data_ptr += strlen("THRID");

		thread_id = *(uint64_t *)data_ptr;
		data_ptr += sizeof(thread_id);

		if (strncmp(data_ptr, "TPCSZ", strlen("TPCSZ")) != 0) {
			query_delete(found);
			return -LESTATUS_IDAT;
		}
		data_ptr += strlen("TPCSZ");

		topic_size = *(uint64_t *)data_ptr;
		data_ptr += sizeof(topic_size);

		if (strncmp(data_ptr, "TPC", strlen("TPC")) != 0) {
			query_delete(found);
			return -LESTATUS_IDAT;
		}
		data_ptr += strlen("TPC");

		thread_topic = malloc(topic_size + 1);
		strncpy(thread_topic, data_ptr, topic_size);
		thread_topic[topic_size] = '\0';
		data_ptr += topic_size;

		thread = lethread_create(thread_topic, thread_id);

		queue_push(found, thread, sizeof(thread));
	}

	return found;
}

status_t parse_response_CMSG(char *raw_data, size_t size) {
	char *data_ptr;


	data_ptr = raw_data;
	if (strncmp(data_ptr, "OK", strlen("OK")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("OK");

	return -LESTATUS_OK;
}

LeMeta * parse_response_META(char *raw_data, size_t size) {
	LeMeta *meta;
	char *data_ptr;
	size_t min_topic_size, max_topic_size;
	size_t min_message_size, max_message_size;
	size_t thread_count;
	size_t version_size;
	char *version;


	data_ptr = raw_data;
	if (strncmp(data_ptr, "MINTPCSZ", strlen("MINTPCSZ")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("MINTPCSZ");

	min_topic_size = *(size_t *)data_ptr;
	data_ptr += sizeof(min_topic_size);

	if (strncmp(data_ptr, "MAXTPCSZ", strlen("MAXTPCSZ")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("MAXTPCSZ");

	max_topic_size = *(size_t *)data_ptr;
	data_ptr += sizeof(max_topic_size);

	if (strncmp(data_ptr, "MINMSGSZ", strlen("MINMSGSZ")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("MINMSGSZ");

	min_message_size = *(size_t *)data_ptr;
	data_ptr += sizeof(min_message_size);
	
	if (strncmp(data_ptr, "MAXMSGSZ", strlen("MAXMSGSZ")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("MAXMSGSZ");

	max_message_size = *(size_t *)data_ptr;
	data_ptr += sizeof(max_message_size);
	
	if (strncmp(data_ptr, "THRN", strlen("THRN")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("THRN");

	thread_count = *(size_t *)data_ptr;
	data_ptr += sizeof(thread_count);

	
	if (strncmp(data_ptr, "VERSZ", strlen("VERSZ")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("VERSZ");

	version_size = *(size_t *)data_ptr;
	data_ptr += sizeof(version_size);
	
	if (strncmp(data_ptr, "VER", strlen("VER")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("VER");

	version = malloc(version_size + 1);
	strncpy(version, data_ptr, version_size);
	version[version_size] = '\0';

	meta = malloc(sizeof(*meta));

	meta->max_topic_size = max_topic_size;
	meta->min_topic_size = min_topic_size;
	meta->max_message_size = max_message_size;
	meta->min_message_size = min_message_size;
	meta->version_size = version_size;
	meta->version = version;
	meta->thread_count = thread_count;

	return meta;
}

status_t parse_response_LIVE(char *raw_data, size_t size) {
	char *data_ptr;


	data_ptr = raw_data;
	if (strncmp(data_ptr, "OK", strlen("OK")) != 0) {
		return -LESTATUS_IDAT;
	}
	data_ptr += strlen("OK");

	return -LESTATUS_OK;
}