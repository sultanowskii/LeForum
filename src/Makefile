CFLAGS = "-I."
LIBS_SERVER = "-pthread"
LIBS_CLIENT = "-pthread"
LIB_FILES_SERVER = $(wildcard lib/*.c)
LIB_FILES_CLIENT = $(wildcard lib/*.c)

server: server/main.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -o server/LeForumServer

client: client/main.c $(LIB_FILES)
	$(CC) $(CFLAGS) $(LIBS_CLIENT) $^ -o server/LeForumClient

test-forum-core: test/test-forum-core.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -g -o test/LeForumCoreTest

test-queue: test/test-queue.c $(LIB_FILES_SERVER)
	$(CC) $(CFLAGS) $(LIBS_SERVER) $^ -g -o test/QueueTest
