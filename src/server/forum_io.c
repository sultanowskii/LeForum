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

FILE *get_lefile(uint64_t lethread_id, char *mode, char *filename, bool_t create) {
	char         path[256];
	FILE        *file;
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

	file = fopen(path, mode);

	return file;
}

status_t lethread_save(LeThread *lethread) {
	size_t  topic_size;
	FILE   *lethread_info_file;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author, -LESTATUS_IDAT)
	NULLPTR_PREVENT(lethread->topic, -LESTATUS_IDAT)

	topic_size = strlen(lethread->topic);

	/** 
	 * This trick clears the file so we don't have to 
	 * have a headache with all these overwriting file stuff
	 */
	lethread_info_file = get_lefile(lethread->id, "wb", FILE_LETHREAD, TRUE);
	fclose(lethread_info_file);

	lethread_info_file = get_lefile(lethread->id, "ab", FILE_LETHREAD, TRUE);

	fwrite(&lethread->id, sizeof(lethread->id), 1, lethread_info_file);
	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, lethread_info_file);
	fwrite(&lethread->first_message_id, sizeof(lethread->first_message_id), 1, lethread_info_file);
	fwrite(&lethread->next_message_id, sizeof(lethread->next_message_id), 1, lethread_info_file);
	fwrite(&topic_size, sizeof(topic_size), 1, lethread_info_file);
	fwrite(lethread->topic, 1, topic_size, lethread_info_file);

	fclose(lethread_info_file);

	return -LESTATUS_OK;
}

status_t lethread_load(LeThread *lethread, uint64_t lethread_id) {
	FILE     *lethread_info_file;
	size_t    topic_size;
	uint64_t  leauthor_id;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	lethread_info_file = get_lefile(lethread_id, "rb", FILE_LETHREAD, FALSE);

	if (lethread_info_file == -LESTATUS_NSFD || lethread_info_file == -LESTATUS_NPTR)
		return lethread_info_file;

	lethread->messages = queue_create(lemessage_delete);
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

	return -LESTATUS_OK;
}

status_t lemessages_save(LeThread *lethread) {
	FILE      *lemessages_file;
	QueueNode *node;
	LeMessage *lemessage;
	size_t     text_size;
	status_t   result;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	result = -LESTATUS_OK;
	node = lethread->messages->first;

	lemessages_file = get_lefile(lethread->id, "wb", FILE_LEMESSAGES, TRUE);
	fclose(lemessages_file);

	lemessages_file = get_lefile(lethread->id, "ab", FILE_LEMESSAGES, TRUE);

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
	size_t  text_size;
	FILE   *lemessages_file;

	NULLPTR_PREVENT(lemessage, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lemessage->text, -LESTATUS_NPTR)

	text_size = strlen(lemessage->text);
	lemessages_file = get_lefile(lemessage->lethread->id, "ab", FILE_LEMESSAGES, TRUE);

	fwrite(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
	fwrite(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
	fwrite(&text_size, sizeof(text_size), 1, lemessages_file);
	fwrite(lemessage->text, 1, text_size, lemessages_file);

	fclose(lemessages_file);

	return -LESTATUS_OK;
}

status_t lemessages_load(LeThread *lethread) {
	FILE      *lemessages_file;
	LeMessage *lemessage;
	size_t     text_size;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->messages, -LESTATUS_NPTR)

	lemessages_file = get_lefile(lethread->id, "rb", FILE_LEMESSAGES, FALSE);
	if (lemessages_file == -LESTATUS_NSFD || lemessages_file == -LESTATUS_NPTR)
		return lemessages_file;

	for (size_t i = 0; i < lethread_message_count(lethread); ++i) {
		lemessage = (LeMessage *)malloc(sizeof(*lemessage));
		fread(&lemessage->id, sizeof(lemessage->id), 1, lemessages_file);
		fread(&lemessage->by_lethread_author, sizeof(lemessage->by_lethread_author), 1, lemessages_file);
		fread(&text_size, sizeof(text_size), 1, lemessages_file);
		text_size = MIN(text_size, MAX_MESSAGE_SIZE);

		lemessage->text = calloc(sizeof(char), text_size + 1);
		fread(lemessage->text, 1, text_size, lemessages_file);

		queue_push(lethread->messages, lemessage, sizeof(lemessage));

		lemessage = nullptr;
	}

	fclose(lemessages_file);

	return -LESTATUS_OK;
}

status_t leauthor_load(LeThread *lethread) {
	LeAuthor *leauthor;
	FILE     *leauthor_file;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)

	leauthor_file = get_lefile(lethread->id, "rb", FILE_LEAUTHOR, FALSE);
	if (leauthor_file == -LESTATUS_NSFD)
		return -LESTATUS_NSFD;

	leauthor = leauthor_create(lethread, FALSE);

	fread(&leauthor->id, sizeof(leauthor->id), 1, leauthor_file);
	fread(leauthor->token, 1, TOKEN_SIZE, leauthor_file);

	fclose(leauthor_file);

	return -LESTATUS_OK;
}

status_t leauthor_save(LeThread *lethread) {
	FILE *leauthor_file;

	NULLPTR_PREVENT(lethread, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author, -LESTATUS_NPTR)
	NULLPTR_PREVENT(lethread->author->token, -LESTATUS_NPTR)

	leauthor_file = get_lefile(lethread->id, "wb", FILE_LEAUTHOR, TRUE);
	fclose(leauthor_file);

	leauthor_file = get_lefile(lethread->id, "ab", FILE_LEAUTHOR, TRUE);

	fwrite(&lethread->author->id, sizeof(lethread->author->id), 1, leauthor_file);
	fwrite(lethread->author->token, TOKEN_SIZE, 1, leauthor_file);

	fclose(leauthor_file);

	return -LESTATUS_OK;
}
