CFLAGS = -I. -Wno-incompatible-pointer-types -Wno-incompatible-pointer-types -Wno-int-conversion
CFLAGS_G = -I. -g -Wall -Wextra

LIBS_SERVER = -pthread
LIBS_CLIENT = -pthread

LIB_FILES_SERVER = $(wildcard lib/*.c server/*.c)
LIB_FILES_CLIENT = $(wildcard lib/*.c client/*.c)
LIB_FILES_TEST_FORUM_CORE = $(wildcard lib/*.c server/forum_io.c)
LIB_FILES_TEST_QUEUE = $(wildcard lib/*.c)
LIB_FILES_TEST_LE_STRCAT = $(wildcard lib/*.c)

c-server: server/server.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $^ $(LIBS_SERVER) -o ../build/LeForumServer

c-client: client/client.c $(LIB_FILES_CLIENT)
	$(CC) $(CFLAGS) $^ $(LIBS_CLIENT) -o ../build/LeForumClient

g-c-server: server/server.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS_G) $^ $(LIBS_SERVER) -Wall -o LeForumServer

g-c-client: client/client.c $(LIB_FILES_CLIENT)
	$(CC) $(CFLAGS_G) $^ $(LIBS_CLIENT) -Wall -o LeForumClient

c-test-forum-core: test/test-forum-core.c $(LIB_FILES_TEST_FORUM_CORE)
	$(CC) $(CFLAGS_G) $^ -o test/LeForumCoreTest

c-test-queue: test/test-queue.c $(LIB_FILES_TEST_QUEUE)
	$(CC) $(CFLAGS_G) $^ -o test/QueueTest

c-test-le-strcat: test/test-le-strcat.c $(LIB_FILES_TEST_LE_STRCAT)
	$(CC) $(CFLAGS_G) $^ -o test/strcatTest

install:
	install -m 755 ../build/LeForumServer /usr/bin/
	install -m 755 ../build/LeForumClient /usr/bin/
