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