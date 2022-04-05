CFLAGS = "-I."

LIBS_SERVER = "-pthread"
LIBS_CLIENT = "-pthread"

LIB_FILES_SERVER = $(wildcard lib/*.c server/core/*.c server/*.c)
LIB_FILES_CLIENT = $(wildcard lib/*.c client/*.c)
LIB_FILES_TEST_FORUM_CORE = $(wildcard server/core/forum.c lib/queue.c lib/shared_ptr.c lib/security.c)
LIB_FILES_TEST_QUEUE = $(wildcard lib/*.c)

server: server/server.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -o server/LeForumServer

client: client/client.c $(LIB_FILES)
	$(CC) $(CFLAGS) $(LIBS_CLIENT) $^ -o server/LeForumClient

test-forum-core: test/test-forum-core.c $(LIB_FILES_TEST_FORUM_CORE)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -g -o test/LeForumCoreTest

test-queue: test/test-queue.c $(LIB_FILES_TEST_QUEUE)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -g -o test/QueueTest
