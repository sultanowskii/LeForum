#include "client/query.h"

ServerQuery * query_create(void * (*parser)(char *raw_data), char *request_data, size_t size) {
	ServerQuery        *query;


	query = malloc(sizeof(*query));
	query->completed = FALSE;
	query->parse_response = parser;
	query->raw_request_data = request_data;
	query->raw_request_data_size = size;

	return query;
}

status_t query_delete(ServerQuery *query) {
	NULLPTR_PREVENT(query, LESTATUS_NPTR)

	if (query->parsed_data != nullptr) {
		free(query->parsed_data);
		query->parsed_data = nullptr;
	}

	if (query->raw_request_data != nullptr) {
		free(query->raw_request_data);
		query->raw_request_data = nullptr;
	}

	free(query);

	return LESTATUS_OK;
}

LeData gen_query_CTHR(const char *topic, size_t size) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("CTHR") + sizeof("TPCSZ") + sizeof(size) + sizeof("TPC") + size;

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "CTHR", sizeof("CTHR"));
	data_ptr += sizeof("CTHR");

	strncpy(data_ptr, "TPCSZ", sizeof("TPCSZ"));
	data_ptr += sizeof("TPCSZ");

	*(size_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TPC", sizeof("TPC"));
	data_ptr += sizeof("TPC");

	strncpy(data_ptr, topic, size);
	data_ptr += size;

	return result;
}

LeData gen_query_GTHR(uint64_t thread_id) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("GTHR") + sizeof("THRID") + sizeof(thread_id);

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "GTHR", sizeof("GTHR"));
	data_ptr += sizeof("GTHR");

	strncpy(data_ptr, "THRID", sizeof("THRID"));
	data_ptr += sizeof("THRID");

	*(uint64_t *)data_ptr = thread_id;
	data_ptr += sizeof(thread_id);

	return result;
}

LeData gen_query_FTHR(const char *topic_part, size_t size) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("FTHR") + sizeof("TPCPSZ") + sizeof(size) + sizeof("TPCP") + topic_part;

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "FTHR", sizeof("FTHR"));
	data_ptr += sizeof("FTHR");

	strncpy(data_ptr, "TPCPSZ", sizeof("TPCPSZ"));
	data_ptr += sizeof("TPCPSZ");

	*(size_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TPCP", sizeof("TPCP"));
	data_ptr += sizeof("TPCP");

	strncpy(data_ptr, topic_part, size);
	data_ptr += size;

	return result;
}

LeData gen_query_CMSG(uint64_t thread_id, const char *msg, size_t size, char *token) {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("CMSG") + sizeof("THRID") + sizeof(thread_id) + sizeof("TXTSZ") + sizeof(size) + sizeof("TXT") + size;
	if (token != nullptr)
		data_size += sizeof("TKN") + TOKEN_SIZE;

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "CMSG", sizeof("CMSG"));
	data_ptr += sizeof("CMSG");

	strncpy(data_ptr, "THRID", sizeof("THRID"));
	data_ptr += sizeof("THRID");

	*(uint64_t *)data_ptr = thread_id;
	data_ptr += sizeof(thread_id);

	strncpy(data_ptr, "TXTSZ", sizeof("TXTSZ"));
	data_ptr += sizeof("TXTSZ");

	*(uint64_t *)data_ptr = size;
	data_ptr += sizeof(size);

	strncpy(data_ptr, "TXT", sizeof("TXT"));
	data_ptr += sizeof("TXT");

	strncpy(data_ptr, msg, size);
	data_ptr += size;

	if (token != nullptr) {
		strncpy(data_ptr, "TKN", sizeof("TKN"));
		data_ptr += sizeof("TKN");

		strncpy(data_ptr, token, TOKEN_SIZE);
		data_ptr += TOKEN_SIZE;
	}

	return result;
}

LeData gen_query_META() {
	LeData result = {0};
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("META");

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "META", sizeof("META"));
	data_ptr += sizeof("META");

	return result;
}

LeData gen_query_META() {
	LeData result;
	char *data_ptr;
	size_t data_size;


	data_size = sizeof("LIVE");

	result.data = malloc(data_size);
	result.size = data_size;
	
	data_ptr = result.data;

	strncpy(data_ptr, "LIVE", sizeof("LIVE"));
	data_ptr += sizeof("LIVE");

	return result;
}