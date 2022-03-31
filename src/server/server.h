#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/time.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>

#include <dirent.h>

#include "lib/constants.h"
#include "lib/status.h"
#include "lib/communication.h"
#include "lib/queue.h"
#include "lib/forum.h"
#include "lib/query.h"

#define PACKET_SIZE 16 * 1024

/*
 * handle_client() argument
 */
struct LeClientInfo {
	int32_t             fd;
	socklen_t           addr_size;
	struct sockaddr_in  addr;
};

status_t                main(int32_t argc, char *argv[]);

size_t                  startup();
void                    cleanup();

void *                  lethread_query_manage();
void *                  lemessages_query_manage();
void *                  lemessage_query_manage();
void *                  leauthor_query_manage();

status_t                s_lethread_save(struct LeThread *lethread);
status_t                s_lemessages_save(struct LeThread *lethread);
status_t                s_lemessage_save(struct LeMessage *lemessage);
status_t                s_leauthor_save(struct LeThread *lethread);

struct LeThread *       s_lethread_create(char *topic, uint64_t lethread_id);
struct LeThread *       lethread_get_by_id(uint64_t lethread_id);

void                    leclientinfo_delete(struct LeClientInfo *clinfo);

void                    signal_handler(const int signum);

uint64_t                next_lethread_id();

void *                  handle_client(void *arg);