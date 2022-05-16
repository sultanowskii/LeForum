#include "server/forum_io.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "global/conf.h"
#include "lib/forum.h"
#include "lib/queue.h"
#include "lib/security.h"
#include "lib/status.h"
#include "lib/util.h"

status_t get_lefile(uint64_t lethread_id, char *mode, char *filename, bool_t create, FILE **file) {
	char         path[256];
	struct stat  st             = {0};

	NULLPTR_PREVENT(mode, -LESTATUS_NPTR)
	NULLPTR_PREVENT(filename, -LESTATUS_NPTR)

	memset(path, 0, sizeof(path));

	sprintf(path, DIR_SERVER "%" SCNu64 "/", lethread_id);

	/* Check if the directory exists */
	if (stat(path, &st) == -1) {
		if (!create)
			return -LESTATUS_NSFD;
		mkdir(path, 0700);
	}

	strcat(path, filename);

	/* Check if the file exists */
	if (stat(path, &st) == -1 && !create)
		return -LESTATUS_NSFD;

	*file = fopen(path, mode);

	return LESTATUS_OK;
}

status_t lethread_save(LeThread *lethread) {
	size_t    topic_size;
	FILE     *lethread_info_file;
	status_t  status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author, -LESTATUS_IDAT)
	NULLPTR_PREVENT(lethread->topic, -LESTATUS_IDAT)

	topic_size = strlen(lethread->topic);

	/** 
	 * This trick clears the file so we don't have to 
	 * have a headache with all these overwriting file stuff
	 */
	get_lefile(lethread->id, "wb", FILE_LETHREAD, TRUE, &lethread_info_file);
	fclose(lethread_info_file);

	status = get_lefile(lethread->id, "ab", FILE_LETHREAD, TRUE, &lethread_info_file);

	fwrite(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, lethread_info_file);
	fwrite(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fwrite(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fwrite(&topic_size, sizeof(topic_size), 1, lethread_info_file);
	fwrite(lethread->topic, 1, topic_size, lethread_info_file);

	fclose(lethread_info_file);

	return LESTATUS_OK;
}

status_t lethread_load(LeThread *lethread, uint64_t lethread_id) {
	FILE     *lethread_info_file;
	size_t    topic_size;
	uint64_t  leauthor_id;
	status_t  status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	status = get_lefile(lethread_id, "rb", FILE_LETHREAD, FALSE, &lethread_info_file);

	if (status == -LESTATUS_NSFD || status == -LESTATUS_NPTR)
		return status;

	queue_create(lemessage_delete, &lethread->messages);
	lethread->author = nullptr;

	fread(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fread(&leauthor_id, sizeof(lethread->author->id), 1, lethread_info_file);
	fread(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fread(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fread(&topic_size, sizeof(topic_size), 1, lethread_info_file);

	topic_size = MIN(topic_size, MAX_TOPIC_SIZE);

	lethread->topic = calloc(sizeof(char), topic_size + 1);

	fread(lethread->topic, 1, topic_size, lethread_info_file);

	fclose(lethread_info_file);

	return LESTATUS_OK;
}

status_t lemessages_save(LeThread *lethread) {
	FILE      *lemessages_file;
	QueueNode *node;
	LeMessage *lemessage;
	size_t     text_size;
	status_t   result;
	status_t   status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	UNUSED(status);

	result = LESTATUS_OK;
	node = lethread->messages->first;

	get_lefile(lethread->id, "wb", FILE_LEMESSAGES, TRUE, &lemessages_file);
	fclose(lemessages_file);

	status = get_lefile(lethread->id, "ab", FILE_LEMESSAGES, TRUE, &lemessages_file);

	while (node != NULL) {
		lemessage = node->data;
		if (lemessage == nullptr || lemessage->text == nullptr) {
			result = -LESTATUS_NPTR;
			continue;
		}

		text_size = strlen(lemessage->text);
		fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
		fwrite(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
		fwrite(&text_size, sizeof(text_size), 1, lemessages_file);
		fwrite(lemessage->text, 1, text_size, lemessages_file);
		node = node->next;
	}

	fclose(lemessages_file);

	return result;
}

status_t lemessage_save(LeMessage *lemessage) {
	size_t    text_size;
	FILE     *lemessages_file;
	status_t  status;

	NULLPTR_PREVENT(lemessage, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lemessage->text, -LESTATUS_NPTR)

	UNUSED(status);

	text_size = strlen(lemessage->text);
	status = get_lefile(lemessage->lethread->id, "ab", FILE_LEMESSAGES, TRUE, &lemessages_file);

	fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
	fwrite(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
	fwrite(&text_size, sizeof(text_size), 1, lemessages_file);
	fwrite(lemessage->text, 1, text_size, lemessages_file);

	fclose(lemessages_file);

	return LESTATUS_OK;
}

status_t lemessages_load(LeThread *lethread) {
	FILE      *lemessages_file;
	LeMessage *lemessage;
	size_t     text_size;
	status_t   status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->messages, -LESTATUS_NPTR)

	status = get_lefile(lethread->id, "rb", FILE_LEMESSAGES, FALSE, &lemessages_file);
	if (status == -LESTATUS_NSFD || status == -LESTATUS_NPTR)
		return status;

	for (size_t i = 0; i < lethread_message_count(lethread); ++i) {
		lemessage = (LeMessage *)malloc(sizeof(*lemessage));
		fread(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
		fread(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
		fread(&text_size, sizeof(text_size), 1, lemessages_file);
		text_size = MIN(text_size, MAX_MESSAGE_SIZE);

		lemessage->text = calloc(sizeof(char), text_size + 1);
		fread(lemessage->text, 1, text_size, lemessages_file);

		queue_push(lethread->messages, lemessage);

		lemessage = nullptr;
	}

	fclose(lemessages_file);

	return LESTATUS_OK;
}

status_t leauthor_load(LeThread *lethread) {
	LeAuthor *leauthor;
	FILE     *leauthor_file;
	status_t  status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	status = get_lefile(lethread->id, "rb", FILE_LEAUTHOR, FALSE, &leauthor_file);
	if (status == -LESTATUS_NSFD)
		return status;

	leauthor_create(lethread, FALSE, &leauthor);

	fread(&leauthor->id, sizeof(leauthor->id), 1, leauthor_file);
	fread(leauthor->token, 1, TOKEN_SIZE, leauthor_file);

	fclose(leauthor_file);

	return LESTATUS_OK;
}

status_t leauthor_save(LeThread *lethread) {
	FILE     *leauthor_file;
	status_t  status;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author->token, -LESTATUS_NPTR)

	UNUSED(status);

	get_lefile(lethread->id, "wb", FILE_LEAUTHOR, TRUE, &leauthor_file);
	fclose(leauthor_file);

	status = get_lefile(lethread->id, "ab", FILE_LEAUTHOR, TRUE, &leauthor_file);

	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, leauthor_file);
	fwrite(lethread->author->token, TOKEN_SIZE, 1, leauthor_file);

	fclose(leauthor_file);

	return LESTATUS_OK;
}
