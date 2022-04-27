CFLAGS = -I. -Wno-incompatible-pointer-types -Wno-incompatible-pointer-types -Wno-int-conversion
CFLAGS_G = -I. -g

LIBS_SERVER = -pthread
LIBS_CLIENT = -pthread

LIB_FILES_SERVER = $(wildcard lib/*.c server/*.c)
LIB_FILES_CLIENT = $(wildcard lib/*.c client/*.c)
LIB_FILES_TEST_FORUM_CORE = $(wildcard lib/forum.c lib/queue.c lib/shared_ptr.c lib/security.c)
LIB_FILES_TEST_QUEUE = $(wildcard lib/*.c)

c-server: server/server.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $^ $(LIBS_SERVER) -o LeForumServer

c-client: client/client.c $(LIB_FILES_CLIENT)
	$(CC) $(CFLAGS) $^ $(LIBS_CLIENT) -o LeForumClient

g-c-server: server/server.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS_G) $^ $(LIBS_SERVER) -o LeForumServer

g-c-client: client/client.c $(LIB_FILES_CLIENT)
	$(CC) $(CFLAGS_G) $^ $(LIBS_CLIENT) -o LeForumClient

c-test-forum-core: test/test-forum-core.c $(LIB_FILES_TEST_FORUM_CORE)
	$(CC) $(CFLAGS) $^ $(LIBS_SERVER) -g -o test/LeForumCoreTest

c-test-queue: test/test-queue.c $(LIB_FILES_TEST_QUEUE)
	$(CC) $(CFLAGS) $^ $(LIBS_SERVER) -g -o test/QueueTest
